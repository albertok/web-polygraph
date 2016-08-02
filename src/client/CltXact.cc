
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/polyLogCats.h"
#include "base/StatIntvlRec.h"
#include "client/BodyParser.h"
#include "client/BodyParserFarm.h"
#include "client/Client.h"
#include "client/ServerRep.h"
#include "client/CltXact.h"
#include "client/CltXactMgr.h"
#include "client/ParseBuffer.h"
#include "csm/ContentMgr.h"
#include "csm/oid2Url.h"
#include "runtime/CompoundXactInfo.h"
#include "runtime/HostMap.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "runtime/PageInfo.h"

CltXact::CltXact(): theOwner(0),
	thePage(0),
	theCompound(0),
	theBodyParser(0),
	theMgr(0) {
	CltXact::reset();
}

void CltXact::reset() {
	Xaction::reset();
	theOwner = 0;

	if (thePage)
		PageInfo::Abandon(thePage);

	if (theCompound)
		CompoundXactInfo::Abandon(theCompound);

	if (theMgr) {
		theMgr->release(this);
		theMgr = 0;
	}

	theSrvRep = 0;
	if (theBodyParser) {
		theBodyParser->farm().put(theBodyParser);
		theBodyParser = 0;
	}
	theNextHighHop = theNextTcpHop = NetAddr();

	theCred.reset();
	theGssContext.free();
	didGssContextAttempt = false;

	theCause = 0;
	theChildCount = 0;
	doRetry = false;
	createdCompound = false;

	theLogCat = lgcCltSide;
}

Agent *CltXact::owner() {
	return theOwner;
}

void CltXact::enqueue() {
	theEnqueTime = TheClock;
}

void CltXact::page(PageInfo *aPage) {
	if (Should(!thePage && aPage))
		thePage = PageInfo::Use(aPage);
}

PageInfo *CltXact::page() {
	return thePage;
}

void CltXact::owner(Client *const anOwner) {
	Must(anOwner);
	Must(!theOwner);
	theOwner = anOwner;
}

void CltXact::exec(Connection *const aConn) {
	Assert(theOid);
	Assert(theOwner);

	start(aConn);
	if (!theOid.foreignUrl()) {
		theSrvRep = TheHostMap->serverRepAt(theOid.viserv());
		theRepContentCfg = TheContentMgr.get(theOid.type());
	}
}

void CltXact::finish(Error err) {
	if (theOid.foreignUrl())
		TheEmbedStats.foreignUrlReceived++;

	if (thePage) {
		thePage->size += theRepSize.actual();
		if (thePage->loneUser() && Should(thePage->start >= 0)) {
			thePage->lifeTime = TheClock - thePage->start;
			Broadcast(ThePageEndChannel, thePage);
		}
		PageInfo::Abandon(thePage);
	}

	Xaction::finish(err);

	if (theBodyParser) {
		theBodyParser->farm().put(theBodyParser);
		theBodyParser = 0;
	}

	theOwner->noteXactDone(this);
}

bool CltXact::controlledMasterRead() {
	if (!Should(theConn))
		return false;

	if (abortIo(&Connection::read))
		return false;

	if (!Should(theConn))
		return false;

	if (theConn->bad()) {
		if (!doRetry) {
			doRetry = theState == stHdrWaiting &&
				!theConn->theRdBuf.contSize() &&
				theConn->useCnt() > 1;
		}
		finish(errOther);
		return false;
	}

	if (theConn->theRdBuf.contSize() > 0 && theFirstRespByteTime < 0)
		theFirstRespByteTime = TheClock - theStartTime;

//IOBuf &buf = theConn->theRdBuf; Comment << here << "read buf: " << endc; printMsg(buf.content(), buf.contSize());

	return true;
}

void CltXact::parse() {
	IOBuf &buf = theConn->theRdBuf;
	const Size availSize = unconsumed();
	const ParseBuffer data = availSize > 0 ?
		ParseBuffer(buf.content(), availSize) : ParseBuffer();

	if (const Size parsedSz = theBodyParser->noteData(data)) {
		consume(parsedSz);
		theConn->theRdBuf.pack();
	}

	if (theRepSize.expectingWhatParsed()) {
		Should(!theRepSize.expected().known());
		theRepSize.expect(theRepSize.actual());
	}
}

void CltXact::checkOverflow() {
	const IOBuf &buf = theConn->theRdBuf;
	if (buf.contSize() >= buf.capacity()) { // full() requires pack()!
		const Size leftoverSize = unconsumed();
		const ParseBuffer leftovers(buf.content(), leftoverSize);
		theBodyParser->noteOverflow(leftovers);
		// if needed, we can let the parser decide how much to chop off
		consume(leftoverSize);
	}
}

// fill buffer space under manager control
bool CltXact::controlledFill(bool &needMore) {
	// theState is usually stConnWaiting or stSpaceWaiting but can also be
	// stHdrWaiting or stBodyWaiting if we write body after HTTP 100 Continue
	WrBuf &buf = theConn->theWrBuf;

	if (!theReqSize.expected().known()) {
		const Size reservedSpace = buf.capacity()/2;
		needMore = buf.spaceSize() < reservedSpace;
		if (needMore)
			return true;

		const char *bodyStart = buf.space();

		try {
			makeReq(buf);
		}
		catch (const Error &err) {
			finish(err);
			return false;
		}

// XXX: remove temporary "here" comments
//Comment << here << "filled buf: " << endc; printMsg(bodyStart, buf.space() - bodyStart);

		if (buf.full()) { // request headers did not fit in the remaining buffer space
			if (ReportError(errReqDontFit)) {
				Comment << "reserved buffer capacity: " << reservedSpace << endc;
				printMsg(bodyStart, buf.space() - bodyStart);
			}
			finish(errOther);
			return false;
		}
	}

//Comment << here << "theReqSize.expected: " << theReqSize.expected() << " actual: " << theReqSize.actual() << " more=" << theReqSize.expectToGetMore() << endc;
	needMore = theReqSize.expectToGetMore();
	return true;
}

// one transaction writes buffer under manager control
bool CltXact::controlledMasterWrite(Size &size) {
	// theState is usually stConnWaiting or stSpaceWaiting but can be
	// stHdrWaiting or stBodyWaiting if we write body after HTTP 100 Continue
	WrBuf &buf = theConn->theWrBuf;
//Comment << here << "writing buf: " << endc; printMsg(buf.content(), buf.contSize());
	Size sz;
	if (buf.contSize() > 0 && abortIo(&Connection::write, &sz))
		return false;

	if (theConn->bad()) {
		Assert(theReqSize.expected() >= 0);
		// HTTP pconn race condition?
		const bool race = buf.contSize() == theReqSize.expected() && theConn->useCnt() > 1;
		doRetry = doRetry || race;
		finish(errOther);
		return false;
	}

	size += sz;
	return true;
}

// possibly many transactions move on, after a single raw write
bool CltXact::controlledPostWrite(Size &size, bool &needMore) {
	// theState is usually stSpaceWaiting but can be stHdrWaiting or
	// stBodyWaiting if we write body after HTTP 100 Continue
	if (size >= theReqSize.expected()) {
		size -= theReqSize.expected();
		needMore = false;
		if (theLastReqByteTime < 0)
			theLastReqByteTime = TheClock - theStartTime;
		if (theState == stSpaceWaiting)
			newState(stHdrWaiting);
		// else continue in its current stHdrWaiting or stBodyWaiting
	} else
		needMore = true;

	return true;
}

void CltXact::controlledAbort() {
	doRetry = true; // retry if other tranactions caused this one to fail
	finish(errPipelineAbort);
}

void CltXact::consume(Size size) {
	theConn->theRdBuf.consumed(size);
	theRepSize.got(size);
}

bool CltXact::expectMore() const {
	if (theConn->atEof())
		return false;

	if (!theRepSize.expected().known())
		return true;

	return theRepSize.expected() > theRepSize.actual() + unconsumed();
}

Size CltXact::unconsumed() const {
	return theRepSize.expectToGet(theConn->theRdBuf.contSize());
}

bool CltXact::validRelOid(const ObjId &oid) const {
	return oid.world() && oid.name() > 0 && 
		TheContentMgr.validId(oid.type());
}

void CltXact::logStats(OLog &ol) const {
	Xaction::logStats(ol);
	ol << theOwner->seqvId();
	ol << (theSrvRep ? theSrvRep->serverIdx() : -1);
}

void CltXact::noteChildNew(CltXact *child) {
	++theChildCount;

	// If we have incomplete compound stats, then we are either
	// CONNECTing or authenticatING. Our first child should update the stats.
	// This assumes that we should not parse 407 body and follow its links.
	if (theCompound && !theCompound->completed() && Should(!child->theCompound)) {
		CompoundXactInfo::Share(theCompound);
		child->theCompound = theCompound;
	}
}

void CltXact::noteChildGone(CltXact *) {
	--theChildCount;
}

void CltXact::noteAbort() {
	newState(stDone);
}

bool CltXact::genCredentials() {
	Must(theOwner);
	Must(!theCred.image());
	return theOwner->credentialsFor(theOid, theCred);
}

bool CltXact::needGssContext() const {
	return false; // only HTTP transactions support GSS-based authentication
}

bool CltXact::initGssContext(const Kerberos::CCache &ccache, const String &target) {
	const Gss::Error err = theGssContext.init(ccache, target);
	if (err && ReportError(errGssContextCreate))
		reportAuthError(aupAssumedProxy, "while obtaining Kerberos TGT", err);
	return !err;
}

Gss::Context &CltXact::gssContext(Connection::NtlmAuth &ntlmState) {
	if (theGssContext)
		ntlmState.gssContext.moveFrom(theGssContext);

	return ntlmState.gssContext;
}

bool CltXact::askedPeer() const {
	Must(false);
	return false;
}

bool CltXact::usePeer() const {
	return false;
}

void CltXact::usePeer(bool) {
	Must(false);
}

void CltXact::noteContent(const ParseBuffer &) {
	Must(false);
}

Error CltXact::noteEmbedded(ReqHdr &) {
	Must(false);
	return 0;
}

void CltXact::noteTrailerHeader(const ParseBuffer &) {
	Must(false);
}

void CltXact::noteEndOfTrailer() {
	Must(false);
}

Error CltXact::noteReplyPart(const RepHdr &) {
	Must(false);
	return 0;
}

bool CltXact::writeFirst() const {
	return true;
}

bool CltXact::startedXactSequence() const {
	if (!theCompound)
		return true; // a sequence of one (this) transaction

	return createdCompound;
}

const CompoundXactInfo *CltXact::partOf() const {
	return theCompound;
}

void CltXact::wakeUp(const Alarm &a) {
	if (!waitingXacts.isolated()) {
		// we are still waiting for the connection slot
		theOwner->dequeSuspXact(this);
	}
	Xaction::wakeUp(a);
}

void CltXact::startReportingAuthError(const char *context, Gss::Error gssErr, const Connection::NtlmAuth *ntlmState) {
	Comment << "authentication error details " << context << endl;

	theOwner->describe(Comment << "\trobot: "); Comment << endl;

	theOwner->lifetimeMessages().print(Comment << "\trobot transaction totals:   ") << endl;
	if (theOwner->lifetimeMessages() != theOwner->periodMessages())
		theOwner->periodMessages().print(Comment<<"\tincluding this busy period: ") << endl;

	if (theConn)
		theConn->print(Comment << "\tconnection: ") << endl;

	Comment << "\torigin server: " << Oid2UrlHost(theOid) << endl;
	if ((theConn && theConn->raddr() != Oid2UrlHost(theOid))
		|| (!theConn && theOwner->proxy(theOid))) {
		Comment << "\tproxy: " << theOwner->proxy(theOid) << endl;
	}

	if (ntlmState && ntlmState->gssContext)
		Comment << "\tKDC used: " << ntlmState->gssContext.kdcAddr() << endl;
	else if (theGssContext.kdcAddr())
		Comment << "\tKDC: " << theGssContext.kdcAddr() << endl;

	if (ntlmState && ntlmState->state != ntlmNone) {
		static std::map<NtlmAuthState, String> stateStage;
		if (stateStage.empty()) {
			stateStage[ntlmSentT1] = "NTLM sent T1";
			stateStage[ntlmSentT3] = "NTLM sent T3";
			stateStage[ntlmDone] = "NTLM done";
			stateStage[ntlmError] = "NTLM error";
		}
		Comment << "\tauthentication stage: " << stateStage[ntlmState->state] << endl;
	}

	if (theCred.image())
		Comment << "\tcredentials used: " << theCred.image() << endl;
	else if(theOwner->credentials())
		Comment << "\tcredentials: " << theOwner->credentials() << endl;

	if (gssErr)
		Comment << gssErr; // prints its own prefix and new lines
}

void CltXact::reportAuthError(AuthPeer, const char *context, Gss::Error gssErr, const Connection::NtlmAuth *ntlmState) {
	startReportingAuthError(context, gssErr, ntlmState);
	Comment << endc;
}
