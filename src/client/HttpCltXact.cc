
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/StatIntvlRec.h"
#include "client/AnyBodyParser.h"
#include "client/Client.h"
#include "client/CltCfg.h"
#include "client/CltDataFilterRegistry.h"
#include "client/CltOpts.h"
#include "client/ChunkedCodingParser.h"
#include "client/HttpCltXact.h"
#include "client/MarkupBodyParser.h"
#include "client/MultiPartParser.h"
#include "client/NtlmAuth.h"
#include "client/SingleCxm.h"
#include "client/PipelinedCxm.h"
#include "client/PrivCache.h"
#include "client/RegExGroups.h"
#include "client/ServerRep.h"
#include "client/UriScriptBodyParser.h"
#include "csm/BodyIter.h"
#include "csm/ContentCfg.h"
#include "csm/oid2Url.h"
#include "runtime/globals.h"
#include "runtime/httpText.h"
#include "runtime/polyErrors.h"
#include "runtime/CompoundXactInfo.h"
#include "runtime/ErrorMgr.h"
#include "runtime/Farm.h"
#include "runtime/HostMap.h"
#include "runtime/HttpCookies.h"
#include "runtime/HttpDate.h"
#include "runtime/LogComment.h"
#include "runtime/PageInfo.h"
#include "runtime/PubWorld.h"
#include "runtime/SharedOpts.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/StatPhaseSync.h"


HttpCltXact::HttpCltXact() {
	HttpCltXact::reset();
}

void HttpCltXact::reset() {
	CltXact::reset();

	theRepHdr.reset();
	theOlcTimes.reset();
	theReqBodySize = -1;
	thePeerState = peerUnknown;
	theSavedRepHeader.reset();
	the100ContinueState = csNone;

	theRangesSize = Size();
	theRangeCount = Size();
	theBodyPartsSize = 0;
	theBodyPartCount = 0;

	theActualRepType = -1;

	theReqFlags = 0;
}

HttpAuthScheme HttpCltXact::proxyAuth() const {
	Assert(theOwner);
	return theOwner->proxyAuthScheme(theOid);
}

PipelinedCxm *HttpCltXact::getPipeline() {
	Assert(theMgr);

	if (!theConn->pipelineable())
		return 0;

	// if we are not already pipelining, create a pipeline manager
	if (!theMgr->pipelining()) {
		Assert(theState == stBodyWaiting);
		PipelinedCxm *mgr = PipelinedCxm::Get();
		mgr->assumeReadControl(this, theMgr);
		theMgr->release(this);
		theMgr = mgr;
		return mgr;
	}

	return dynamic_cast<PipelinedCxm*>(theMgr);
}

void HttpCltXact::pipeline(PipelinedCxm *aMgr) {
	Assert(theMgr != aMgr); // or we will overincrement useLevel
	theMgr->release(this);
	aMgr->join(this); // we also call control() in exec()
	theMgr = aMgr;
	theConn = theMgr->conn();
	Assert(theConn);
	theConn->startUse();
}

void HttpCltXact::exec(Connection *const aConn) {
	CltXact::exec(aConn);

	theOwner->cfg()->selectAbortCoord(theAbortCoord);

	Should (!theConn->tunneling() || theConn->sslConfigured());
	if (theConn->sslConfigured() && !theConn->sslActive()) {
		if (theConn->tunneling())
			theOid.connect(true);
		else
			theConn->sslActivate();
	}

	if (!theMgr)
		theMgr = SingleCxm::Get(); // may be changed to PipelinedCxm later
	theMgr->control(this);

	newState(stConnWaiting);
}

void HttpCltXact::finish(Error err) {
	if (!err && theRepHdr.redirect())
		redirect();

	// special actions for objects with bodies
	if (!theOid.head() && theBodyParser && theBodyParser->used()) {
		// check MD5
		if (!err && !theOid.aborted() && theRepHdr.theChecksum.set()) {
			theCheckAlg.final();
			if (!theCheckAlg.sum().equal(theRepHdr.theChecksum)) {
				if (ReportError(errChecksum)) {
					theRepHdr.theChecksum.print(Comment << "MD5 expected: ") << endc;
					theCheckAlg.sum().print(Comment << "MD5 received: ") << endc;
					if (TheOpts.theDumpFlags(dumpErr, dumpAny))
						printMsg(theConn->theRdBuf);
				}
				err = errOther;
			}
		}

		// cache
		if (!err && theOid.cachable()) {
			if (PrivCache *cache = theOwner->privCache())
				cache->storeOid(theOid);
		}
	}

	if (err && theOwner->privCache())
		theOwner->privCache()->purgeOid(theOid);

	if (!theMgr) {
		// no manager if lifetime timeout happens while in waiting queue
		Must(err);
		Must(err == errXactLifeTimeExpired);
	} else if (err) {
		if (err != errPipelineAbort)
			theMgr->noteAbort(this);
	} else {
		theMgr->noteDone(this);
	}

	if (theAuthXact) {
		++theAuthXact->exchanges;
		theAuthXact->reqSize += theReqSize.actual();
		theAuthXact->repSize += theRepSize.actual();
		if (theHttpStatus != RepHdr::sc407_ProxyAuthRequired || theError) {
			theAuthXact->lifeTime = TheClock - theAuthXact->startTime;
			theAuthXact->final = true;
		}
	}

	if (theOid.aborted())
		theActualRepType = TheUnknownContentId;

	CltXact::finish(err);
}

// XXX: the needMore value and the return value are unused
bool HttpCltXact::controlledPostRead(bool &needMore) {
	if (theState == stHdrWaiting) {
		const Error err = getHeader();
		if (err || (theOid.connect() && theState == stBodyWaiting)) {
			finish(err);
			return false;
		}
	}

	if (theState == stBodyWaiting) {
		getBody();
		return false;
	}

	return false;
}

Error HttpCltXact::getHeader() {
	Assert(theState == stHdrWaiting);

	if (theRepHdr.parse(theConn->theRdBuf.content(), theConn->theRdBuf.contSize())) {
		Error err = interpretHeader();

		if (theRepHdr.theStatus != RepHdr::sc100_Continue) {
			// dump reply header after interpretHeader() to dump more oid flags
			static int respCount = 0;
			if (!respCount++ || TheOpts.theDumpFlags(dumpRep, dumpHdr))
				printMsg(theConn->theRdBuf, theRepHdr.theHdrSize);

			if (!err)
				err = handleAuth();
			if (!err) {
				consume(theRepHdr.theHdrSize);
				if (theHttpStatus == RepHdr::sc407_ProxyAuthRequired)
					saveRepHeader();
				newState(stBodyWaiting);
			}
		} else {
			if (!err) {
				consume(theRepHdr.theHdrSize);
				Assert(the100ContinueState == csWaiting);
				the100ContinueState = csDone;
				theMgr->resumeWriting(this);
			}
			theRepHdr.reset();
			theRepSize.reset();
			Assert(theState == stHdrWaiting);
		}

		return err;
	} else
	if (expectMore()) {
		if (theConn->theRdBuf.full())
			return errHugeHdr;
		else
			return 0;
	} else {
		const bool readNothing = !theConn->theRdBuf.contSize();
		// HTTP pconn race condition?
		const bool race = readNothing && theConn->useCnt() > 1;
		doRetry = doRetry || race;
		if (race)
			return errOther;
		else
			return readNothing ? errNoHdrClose : errPrematureEoh;
	}
	Should(false); // not reached
	return errOther;
}

void HttpCltXact::getBody() {
	Assert(theState == stBodyWaiting);
	Assert(theBodyParser);

	parse();

	if (expectMore()) {
		checkOverflow();
		return;
	}

	const Size leftoverSize = unconsumed();

	// no more data, set expected size if it was unknown
	if (!theRepSize.expected().known())
		theRepSize.expect(theRepSize.actual() + leftoverSize);

	const bool pgAborted = cfgAbortedReply();

	// make sure we do not leave anything behind and complain if needed
	if (leftoverSize > 0) {
		if (pgAborted) {
			// no need to complain, a Polygraph-initiated abort
		} else
		if (theRepSize.actual() + leftoverSize < theRepSize.expected()) {
			// no need for parser to complain since it is not a content error
		} else {
			const ParseBuffer leftovers(theConn->theRdBuf.content(),
				leftoverSize);
			theBodyParser->noteLeftovers(leftovers);
		}
		consume(leftoverSize);
	}

	if (theRepSize.expected() == theRepSize.actual()) {
		finish(0);
		return;
	}

	if (theRepSize.expected() < theRepSize.actual()) {
		finish(errExtraRepData);
		return;
	}

	Assert(theRepSize.expected() > theRepSize.actual());

	if (pgAborted) {
		theConn->lastUse(true);
		theOid.aborted(true);
		finish(0); // not an error, configuration told server to abort
		return;
	}

	finish(errPrematureEof);
}

bool HttpCltXact::controlledFill(bool &needMore) {
	if (!CltXact::controlledFill(needMore))
		return false;

	if (the100ContinueState != csWaiting && theReqSize.expectToGetMore()) {
		WrBuf &buf = theConn->theWrBuf;
		// append request body if needed
		const char *bodyStart = buf.space();
		if (theBodyIter && !theBodyIter->pour())
			return false;
		theReqSize.got(buf.space() - bodyStart);
		if (TheOpts.theDumpFlags(dumpReq, dumpBody))
			printMsg(bodyStart, buf.space() - bodyStart);
	}

	return true;
}

// possibly many transactions move on, after a single raw write
bool HttpCltXact::controlledPostWrite(Size &size, bool &needMore) {
	CltXact::controlledPostWrite(size, needMore);

	if (!needMore) {
		Assert(the100ContinueState != csWaiting);
		if (theOid.foreignUrl())
			TheEmbedStats.foreignUrlRequested++;
	} else
	if (the100ContinueState == csWaiting) {
		Assert(theState == stSpaceWaiting);
		// We do not need to write more if we wrote the headers.
		// HttpCltXact::controlledFill does not fill body in csWaiting.
		if (theConn->theWrBuf.empty()) { // wrote all headers, now need to wait
			needMore = false; // but will resumeWriting()
			newState(stHdrWaiting);
		}
	}

	return true;
}

void HttpCltXact::makeReq(WrBuf &buf) {
	ofixedstream os(buf.space(), buf.spaceSize());

	if (theState == stConnWaiting) {
		if (theOid.connect())
			makeConnectReq(os);
		else
			makeExplicitReq(os);
		newState(stSpaceWaiting);
	}
}

// make a CONNECT request
void HttpCltXact::makeConnectReq(ostream &os) {
	os << rlpConnect;
	Oid2UrlHost(theOid, true, os);
	makeReqVersion(os);
	makeHopByHopHdrs(os);

	static int reqCount = 0;
	finishReqHdrs(os, !reqCount++);

	// no body for CONNECT requests
	theReqOid.type(TheBodilessContentId);
}

// make a non-CONNECT request
void HttpCltXact::makeExplicitReq(ostream &os) {
	Assert(the100ContinueState == csNone);
	Assert(!theReqContentCfg);
	Assert(!theBodyIter);
	// decide whether the request should have a body
	if (theOid.post() || theOid.put()) {
		theReqSize.expectedBody(true);
		theReqContentCfg = theOwner->selectReqContent(theOid, theReqOid);
		theBodyIter = theReqContentCfg->getBodyIter(theReqOid);
		theBodyIter->start(&theConn->theWrBuf);
		theReqBodySize = theBodyIter->contentSize();

		static RndGen rng;
		if (theReqBodySize > 0 &&
			((theOwner->cfg()->theReqBodyPauseProb >= 0 &&
			rng.event(theOwner->cfg()->theReqBodyPauseProb)) ||
			(theOwner->cfg()->theReqBodyPauseStart >= 0 &&
			theReqBodySize >= theOwner->cfg()->theReqBodyPauseStart.byte())))
				the100ContinueState = csWaiting;
	} else
		theReqOid.type(TheBodilessContentId);

	// make headers
	{
		makeReqMethod(os);
		makeEndToEndHdrs(os);
		makeHopByHopHdrs(os);
		makeCookies(os); // last, to know buffer space left
		static int reqCount = 0;
		finishReqHdrs(os, !reqCount++);
	}

	// where we should abort
	theAbortSize = theAbortCoord.pos(theReqSize.header(), theReqBodySize);
}

void HttpCltXact::finishReqHdrs(ostream &os, bool forceDump) {
	IOBuf &buf = theConn->theWrBuf;
	const char *reqStart = buf.space();
	buf.appended((streamoff)os.tellp());

	// give filters a chance, they may add their own headers
	TheCltDataFilterRegistry().apply(this, buf);

	buf.append("\r\n", Min(buf.spaceSize(), Size(2))); // end-of-header

	Size fullSize = buf.space() - reqStart;
	theReqSize.header(fullSize);
	if (theReqBodySize.known())
		fullSize += theReqBodySize;
	theReqSize.expect(fullSize);
	theReqSize.got(buf.space() - reqStart);

	// dump request header
	if (forceDump || TheOpts.theDumpFlags(dumpReq, dumpHdr))
		printMsg(reqStart, buf.space() - reqStart);
}

void HttpCltXact::makeReqMethod(ostream &os) {
	if (theOid.get())
		os << rlpGet;
	else
	if (theOid.post())
		os << rlpPost;
	else
	if (theOid.head())
		os << rlpHead;
	else
	if (theOid.put())
		os << rlpPut;
	else
		Assert(false);
}

void HttpCltXact::makeReqVersion(ostream &os) {
	if (theOwner->httpVersion() <= HttpVersion(1,0))
		os << rlsHttp1p0;
	else
		os << rlsHttp1p1;
}

void HttpCltXact::makeEndToEndHdrs(ostream &os) {

	// send full URL only if we are talking directly to a proxy
	if (theOwner->proxy(theOid) && !theConn->tunneling())
		Oid2Url(theOid, os);
	else
		Oid2UrlPath(theOid, os);

	makeReqVersion(os);
	
	/* request-header fields */

	os << hfAccept;
	if (const String *codings = theOwner->cfg()->theAcceptedContentCodings)
		os << hfpAcceptEncoding << *codings << crlf;

	os << hfpHost;
	Oid2UrlHost(theOid, false, os);
	os << crlf;

	if (theOid.imsAny() && olcTimes().lmt() >= 0) {
		const Time t = theOid.ims200() ?
			(olcTimes().lmt() - Time::Sec(1)) : 
			(olcTimes().lmt() + Time::Sec(1));
		HttpDatePrint(os << hfpIMS, t) << crlf;
		theReqFlags |= xfValidation;
	}

	if (theOid.reload())
		os << hfReload;

	if (theOid.range()) {
		RangeCfg::RangesInfo res = theOwner->makeRangeSet(os, theOid, *theRepContentCfg);
		theRangeCount = res.theCount;
		theRangesSize = res.theTotalSize;
	}

	/* entity-header fields */

	os << hfpXXact << TheGroupId << ' ' << theId << ' ' << hex << theReqFlags << dec << crlf;

	// send public world info
	if (!theOid.foreignUrl()) {
		if (const HostCfg *host = TheHostMap->at(theOid.viserv())) {
			const PubWorld &pubWorld = *host->thePubWorld;
			os << hfpXLocWorld << pubWorld.localSlice() << crlf;

			if (const PubWorldSlice *slice = pubWorld.sliceToSync())
				os << hfpXRemWorld << *slice << crlf;
		}

		if (theSrvRep)
			os << hfpXTarget << theSrvRep->addr() << crlf;
	}

	os << hfpXAbort << theAbortCoord.whether() 
		<< ' ' << theAbortCoord.where() << crlf;

	// report our readiness to change phase
	os << hfpXPhaseSyncPos << TheStatPhaseMgr.phaseSyncPos() << crlf;

	if (theReqContentCfg &&
		theReqContentCfg->calcChecksumNeed(theReqOid))
		putChecksum(*theReqContentCfg, theReqOid, os);

	if (theBodyIter) {
		theBodyIter->putHeaders(os);
		const String &pfx(theReqContentCfg->url_pfx(theReqOid.hash()));
		const String &ext(theReqContentCfg->url_ext(theReqOid.hash()));
		if (pfx || ext)
			os
				<< hfpContDisposition << '"'
				<< pfx << theReqOid.name() << ext
				<< '"' << crlf;
	}

	Assert(the100ContinueState != csDone);
	if (the100ContinueState == csWaiting)
		os << hfExpect100Continue;

	makeOriginAuthHdr(os);
}

void HttpCltXact::makeHopByHopHdrs(ostream &os) {
	/* general-header fields */
	// persistency indication depends on HTTP version
	if (theOwner->httpVersion() <= HttpVersion(1,0)) {
		if (theConn->reusable())
			os << (theOwner->proxy(theOid) ? hfConnAlivePxy : hfConnAliveOrg);
	} else {
		if (!theConn->reusable())
			os << (theOwner->proxy(theOid) ? hfConnClosePxy : hfConnCloseOrg);
	}

	makeProxyAuthHdr(os);
}

void HttpCltXact::makeCookies(ostream &os) {
	HttpCookies *const cookies(theOwner->cookies(theOid));

	if (theOwner->doCookies()) {
		// configure future response parser to collect new cookies
		theRepHdr.collectCookies(cookies);
	}

	// send back all cookies we kept (if any)
	if (!cookies)
		return; // but we may collect some from the response

	Assert(theOwner->doCookies());

	for (const HttpCookie *cookie = cookies->first();
		cookie;
		cookie = cookies->next()) {

		WrBuf &buf = theConn->theWrBuf;
		const Size usedSize = (streamoff)os.tellp();
		const Size headerSize =
			hfpCookie.len() + cookie->data().len() + 4;
		const Size spaceRemaining = buf.spaceSize() - usedSize;

		if (headerSize < spaceRemaining)
			os << hfpCookie << cookie->data() << crlf;
		else
		if (ReportError(errCookiesDontFit)) {
			Comment << "header size: " << headerSize
				<< "; space left: " << spaceRemaining << endc;
		}
	}
}

void HttpCltXact::makeProxyAuthHdr(ostream &os) {
	const HttpAuthScheme scheme(theOwner->proxyAuthScheme(theOid));
	Connection::NtlmAuth &ntlmState(theConn->theProxyNtlmState);
	makeAuthHdr(hfpProxyAuthorization, scheme, ntlmState, os);
}

void HttpCltXact::makeOriginAuthHdr(ostream &os) {
	const HttpAuthScheme scheme(theOwner->originAuthScheme(theOid));
	Connection::NtlmAuth &ntlmState(theConn->theOriginNtlmState);
	makeAuthHdr(hfpAuthorization, scheme, ntlmState, os);
}

void HttpCltXact::makeAuthHdr(const String &header, const HttpAuthScheme scheme, Connection::NtlmAuth &ntlmState, ostream &os) {
	theOid.authCred(false);
	switch (ntlmState.state) {
		case ntlmNone: { // Initiating auth
			if (scheme == authNtlm) {
				makeAuthorization(header, scheme, os);
				NtlmAuthPrintT1(os);
				os << crlf;
				ntlmState.state = ntlmSentT1;
			} else
			if (scheme == authNegotiate) {
				makeAuthorization(header, scheme, os);
				NegoNtlmAuthPrintT1(os, ntlmState.useSpnegoNtlm);
				os << crlf;
				ntlmState.state = ntlmSentT1;
			} else
			if (scheme == authBasic) {
				if (theOwner->credentialsFor(theOid, theCred)) {
					makeAuthorization(header, scheme, os);
					PrintBase64(os, theCred.image().data(), theCred.image().len());
					os << crlf;
				}
			} else {
				Should(scheme == authNone);
				// no header, we have not been asked to authenticate
			}
			break;
		}
		case ntlmSentT1: {
			if (theOwner->credentialsFor(theOid, theCred)) {
				Area aUser = theCred.name();
				String sUser(aUser.data(), aUser.size());
				Area aPass = theCred.password();
				String sPass(aPass.data(), aPass.size());
				makeAuthorization(header, scheme, os);
				if (NegoNtlmAuthPrintT3(os,
					ntlmState.hdrRcvdT2.cstr(),
					sUser.cstr(),
					sPass.cstr(),
					ntlmState.useSpnegoNtlm)) {
					os << crlf;
					ntlmState.state = ntlmSentT3;
				} else {
					os << crlf;
					ntlmState.state = ntlmError;
				}
			} else {
				ntlmState.state = ntlmError;
			}
			break;
		}
		case ntlmDone:
		case ntlmError: {
			// do nothing
			break;
		}
		default: {
			Should(false);
			ntlmState.state = ntlmError;
		}
	}
}

void HttpCltXact::makeAuthorization(const String &header, const HttpAuthScheme scheme, ostream &os) {
	os << header;
	switch (scheme) {
		case authNtlm:
			os << "NTLM ";
			break;
		case authNegotiate:
			os << "Negotiate ";
			break;
		default:
			os << "Basic ";
	}
}

Error HttpCltXact::interpretHeader() {
	theOwner->absorbCookies(theOid, theRepHdr.theCookies);
	theRepSize.header(theRepHdr.theHdrSize);

	// does reply line make sense?
	if (!theRepHdr.theHttpVersion.known() || theRepHdr.theStatus < 0)
		return errHttpRLine;

	if (100 <= theRepHdr.theStatus && theRepHdr.theStatus < 200) {
		if (theRepHdr.theStatus == RepHdr::sc100_Continue) {
			theContinueMsgTime = TheClock - theStartTime;
			return the100ContinueState == csWaiting ? 0 : errUnexpected100Continue;
		} else
			return errUnsupportedControlMsg;
	}

	if (theOid.connect() && theRepHdr.theStatus == RepHdr::sc200_OK) {
		theRepSize.expect(theRepHdr.theHdrSize);
		theActualRepType = TheBodilessContentId;
		return 0;
	}

	if (const Error err = setStatusCode(theRepHdr.theStatus))
		return err;

	if (theSrvRep) // TODO: why is this done here and not earlier or later?
		theSrvRep->noteRequest();

	// check content-length, set RepSize if possible
	if (theRepHdr.expectBody()) {
		if (theOid.head()) {
			theActualRepType = TheBodilessContentId;
			theRepSize.expect(theRepHdr.theHdrSize);
		} else {
			if (theRepHdr.theStatus == RepHdr::sc200_OK)
				theActualRepType = theOid.type();
			else
				theActualRepType = TheUnknownContentId;
			theRepSize.expectedBody(true);
			if (theRepHdr.theTransferEncoding == MsgHdr::tcChunked) {
				if (theRepHdr.theContSize >= 0) // MUST ignore
					ReportError(errChunkedButCLen);
			} else
			if (theRepHdr.theContSize >= 0)
				theRepSize.expect(theRepHdr.theHdrSize + theRepHdr.theContSize);
			else
			if (theRepHdr.persistentConnection() &&
				!theRepHdr.multiRange())
				return errPersistButNoCLen;
		}
	} else {
		theActualRepType = TheBodilessContentId;
		if (theRepHdr.theContSize >= 0)
			return errUnexpectedCLen;
		theRepSize.expect(theRepHdr.theHdrSize);
	}

	if (theRepHdr.theStatus == RepHdr::sc206_PartialContent) {
		if (theRepHdr.theContRangeFirstByte >= 0 &&
			theRepHdr.theContRangeLastByte >= 0) {
			if (theRepHdr.theContSize + theRepHdr.theContRangeFirstByte - theRepHdr.theContRangeLastByte != 1)
				return errPartContBadByteRange;
			if (theRepHdr.theContRangeInstanceLength >= 0 &&
				theRepHdr.theContRangeInstanceLength < theRepHdr.theContSize)
				return errPartContBadInstanceLen;
		}
	} else
	if (theRepHdr.theStatus != RepHdr::sc416_RequestedRangeNotSatisfiable) {
		if (theRepHdr.theContRangeFirstByte >= 0 ||
			theRepHdr.theContRangeLastByte >= 0 ||
			theRepHdr.theContRangeInstanceLength >= 0)
			return errUnexpectedCRange;
	}

	// check that we can handle transfer encoding
	if (theRepHdr.theTransferEncoding == MsgHdr::tcOther)
		return errUnknownTransferEncoding;

	theOid.repToRedir(theRepHdr.redirect());

	if (!theOid.connect() &&
		!theRepHdr.polyHeaders() &&
		theRepHdr.expectPolyHeaders()) {
		if (const HostCfg *host = TheHostMap->at(theOid.viserv())) {
			// do not expect Polygraph-specific headers from non-HTTP origins
			if (host->theProtocol == Agent::pHTTP) {
				theOid.foreignSrc(true);
				noteError(errForeignSrc);
			}
		}
	}

	checkAcl();

	// firstHand here means our request reached the server and
	// server's reply reached us
	const bool firstHand = id().myMutant(theRepHdr.theXactId);

	theOid.cachable(theRepHdr.isCachable);
	theOid.hit(!firstHand && theRepHdr.theXactId && theOid.basic());

	if (theOid.hit()) {
		if (!theOid.offeredHit() && !TheCltOpts.ignoreFalseHits)
			noteError(errFalseHit);

		if (!theOid.cachable())
			noteError(errUnchbHit);
	}

	if (firstHand) { // native miss
		checkDateSync();
		firstHandSync();
	} else
	if (theRepHdr.theXactId) { // native second-hand response
		if (theOid.reload())
			noteError(errReloadHit);
	} else
	if (theRepHdr.theDate >= 0 && theRepHdr.theDate < HttpDateAtMost(theStartTime)) {
		// our response is aged: it was generated before we asked for it
		if (theOid.reload())
			noteError(errReloadHit);
	}

	if (theOid.offeredHit() && !theOid.hit())
		noteError(errFalseMiss);

	checkFreshness();

	// note if the server will close the connection
	if (!theRepHdr.persistentConnection()) {
		theConn->lastUse(true);
		theMgr->noteLastXaction(this);
	}

	theBodyParser = selectBodyParser();
	theRepFlags = theRepHdr.theXactFlags;

	return 0;
}

void HttpCltXact::noteError(const Error &err) {
	if (err == errFalseMiss) {
		if (TheCltOpts.printFalseMisses) {
			Oid2Url(theOid, cout << "False-Miss: ");
			cout << endl;
			printMsg(theConn->theRdBuf, theRepHdr.theHdrSize);
		}
		return; // do not report false misses; they may not be errors
	}

	if (err == errForeignSrc) {
		if (theOid.foreignUrl())
			return; // response to a foreign URL may be foreign

		static bool informed = false;
		if (!informed) {
			Comment(5) << "fyi: received first foreign response to " <<
				"Polygraph-specific URL:" << endc;
			printMsg(theConn->theRdBuf);
			informed = true;
		}

		if (theHttpStatus == RepHdr::sc200_OK && theOwner->cfg()->acl())
			return; // let checkAcl() handle this case

		if (TheOpts.acceptForeignMsgs)
			return; // the user told us to accept all foreign responses
	}

	if (!ReportError(err))
		return;

	/* supply additional information */

	if (err == errStaleHit || err == errReloadHit) {
		HttpDatePrint(cout << "\trep: ", theRepHdr.theDate) << endl;
		HttpDatePrint(cout << "\tlmt: ", olcTimes().lmt()) << endl;
		HttpDatePrint(cout << "\treq: ", theStartTime) << endl;
		HttpDatePrint(cout << "\tnow: " ) << endl;
		HttpDatePrint(cout << "\texp: ", olcTimes().exp()) << endl;
	}

	if (TheOpts.theDumpFlags(dumpErr, dumpAny))
		printMsg(theConn->theRdBuf);
}

void HttpCltXact::saveRepHeader() {
	theConn->theRdBuf.copyContent(theSavedRepHeader, theRepHdr.theHdrSize);
}

void HttpCltXact::firstHandSync() {
	// update public world info
	if (theRepHdr.theRemWorld)
		updatePubWorld(theRepHdr.theRemWorld);

	if (theSrvRep)
		theSrvRep->noteFirstHandResponse();

	// sync phases
	if (theRepHdr.thePhaseSyncPos >= 0 && theRepHdr.theGroupId)
		TheStatPhaseSync.notePhaseSync(theRepHdr.theGroupId, theRepHdr.thePhaseSyncPos);
}

Error HttpCltXact::doForbidden() {
	Error err;
	if (theOwner->hasCredentials()) {
		if (theCred.image()) { // authed
			if (theCred.valid())
				err = errForbiddenAfterAuth;
		} else
		if (theAuthXact) // authing
			err = errForbiddenDuringAuth;
		else
			err = errForbiddenBeforeAuth;
	} else
		err = errForbiddenWoutCreds;
	return err;
}

Error HttpCltXact::doProxyAuth() {
	const bool needed(theHttpStatus == RepHdr::sc407_ProxyAuthRequired);
	Connection::NtlmAuth &ntlmState(theConn->theProxyNtlmState);
	const Error err(doAuth(true, needed, ntlmState));
	if (err && ReportError(err)) {
		Comment << "robot: " << theOwner->host()
			<< " credentials: " << theCred.image()
			<< " proxy: " << theOwner->proxy(theOid) << endc;
	}
	return err;
}

Error HttpCltXact::doOriginAuth() {
	Error err;
	// do no origin authentication during proxy authentication
	if (theHttpStatus != RepHdr::sc407_ProxyAuthRequired) {
		const bool needed(theHttpStatus == RepHdr::sc401_Unauthorized);
		Connection::NtlmAuth &ntlmState(theConn->theOriginNtlmState);
		err = doAuth(false, needed, ntlmState);
		if (err && ReportError(err)) {
			Comment << "robot: " << theOwner->host()
				<< " credentials: " << theCred.image()
				<< " origin: " << Oid2UrlHost(theOid) << endc;
		}
	}
	return err;
}

// For fresh connection auth scheme is determined by proxy response headers.
// If NTLM or Negotiate auth is started for a connection it never changes later.
// If a proxy changes auth scheme, all compound auth xactions that have not yet
// finished authenticating will fail. Other xactions will not be affected.
Error HttpCltXact::doAuth(const bool proxyAuth, const bool needed, Connection::NtlmAuth &ntlmState) {
	Error authError;

	if (needed) { // denied
		AuthChallenge &challenge(proxyAuth ?
			theRepHdr.theProxyAuthenticate :
			theRepHdr.theOriginAuthenticate);
		if (challenge.scheme == authNone)
			return proxyAuth ? errProxyAuthHeaders : errOriginAuthHeaders;
		if (!theOwner->hasCredentials())
			return proxyAuth ? errProxyAuthWoutCreds : errOriginAuthWoutCreds;
		switch (ntlmState.state) {
			case ntlmNone: {
				if (challenge.scheme == authNtlm) {
					// sent nothing, proxy should signal disconnect, 
					// NTLM authentication will resume once we reconnect
					doRetry = true;
					authError = startAuth(authNtlm);
				} else
				if (challenge.scheme == authNegotiate) {
					ntlmState.useSpnegoNtlm = isSpnegoNtlm(challenge.params.cstr());
					if (ntlmState.useSpnegoNtlm && theOwner->cfg()->theSpnegoAuthRatio >= 0) {
					     static RndGen rng;
					     ntlmState.useSpnegoNtlm = rng.event(theOwner->cfg()->theSpnegoAuthRatio);
					}
					
					// According to rfc4559 spnego-based negotiate auth is not supposed
					// to be used by proxies (but can be passed transparently to servers),
					// so we can't really say whether we need to retry here or what. Need
					// to test on available clients and proxies.
					doRetry = ntlmState.useSpnegoNtlm;
					
					// sent nothing, in case of NTLMSSP proxy should signal disconnect, 
					// Negotiate authentication will resume once we reconnect
					authError = startAuth(authNegotiate);
				} else
				if (!theCred.image()) {
					// sent nothing, Basic auth
					doRetry = true;
					authError = startAuth(authBasic);
				} else
				if (theCred.valid()) { // sent valid Basic auth
					authError = proxyAuth ? errProxyAuthAfterAuth : errOriginAuthAfterAuth;
				}
				// else auth correctly denied due to invalid credentials
				break;
			}
			case ntlmSentT1: {
				ntlmState.hdrRcvdT2 = challenge.params;
				doRetry = true; // continue processing in HopByHops
				break;
			}
			case ntlmSentT3: { // auth failed
				ntlmState.state = ntlmError;
				if (!theCred.valid()) // sent invalid NTLM auth
					break;
				// fall through
			}
			default: { // errors and weird cases
				authError = proxyAuth ? errProxyAuthAfterAuth : errOriginAuthAfterAuth;
			}
		}
	} else { // allowed
		switch (ntlmState.state) {
			case ntlmNone: {
				// succeeded at Basic Auth
				if (theCred.image() && !theCred.valid()) { // sent invalid
					authError = proxyAuth ? errProxyAuthAllowed : errOriginAuthAllowed;
				}
				break;
			}
			case ntlmSentT3: { // as expected
				ntlmState.state = ntlmDone;
				if (!theCred.valid())
					authError = proxyAuth ? errProxyAuthAllowed : errOriginAuthAllowed;
				break;
			}
			case ntlmDone: {
				// do nothing, everything is ok
				break;
			}
			default: { // fall though for all weird cases
				authError = proxyAuth ? errProxyAuthAllowed : errOriginAuthAllowed;
			}
		}
	}
	return authError;
}

Error HttpCltXact::startAuth(const HttpAuthScheme scheme) {
	if (theAuthXact)
		return errAuthBug;

	theAuthXact = CompoundXactInfo::Create();
	theAuthXact->startTime = theStartTime;
	if (theHttpStatus == RepHdr::sc401_Unauthorized)
		theOwner->noteOriginAuthReq(this, scheme);
	else
		theOwner->noteProxyAuthReq(this, scheme);
	return 0;
}

Error HttpCltXact::handleAuth() {
	if (theHttpStatus == RepHdr::sc403_Forbidden)
		return doForbidden();

	Error err = doProxyAuth();
	if (!err)
		err = doOriginAuth();
	return err;
}

void HttpCltXact::redirect() {
	ObjId dest = theRepHdr.theLocn.oid;
	dest.rediredReq(true);
	dest.get(true);

	if (!dest.foreignUrl()) {
		if (!validRelOid(dest)) {
			ReportError(errRedirLocation);
			return;
		}

		if (theRepHdr.theLocn.host) {
			if (const Error err = setViserv(theRepHdr.theLocn.host, dest)) {
				noteError(err);
				return;
			}
		} else {
			dest.viserv(theOid.viserv());
		}
	
		theOwner->selectTarget(dest);
		dest.repeat(true); // polysrv only redirects to seen URLs
	} else {
		dest.type(TheForeignContentId);
		dest.repeat(false); // XXX: may be a repeat, we do not know
	}
	// XXX: hot() is unset

	theOwner->noteRedirect(this, dest);
}

BodyParser *HttpCltXact::selectBodyParser() {
	BodyParser *cparser = selectContentParser();

	if (theRepHdr.chunkedEncoding())
		return ChunkedCodingParser::GetOne(this, cparser);
	if (theRepHdr.multiRange())
		return MultiPartParser::GetOne(this, cparser, theRepHdr.theBoundary);

	return cparser;
}

BodyParser *HttpCltXact::selectContentParser() {
	// do not parse if we are not going to request embedded objects
	if (theOwner->cfg()->theEmbedRecurRatio <= 0)
		return AnyBodyParser::GetOne(this);

	// parse if content is markup
	if (theRepHdr.markupContent())
		return selectMarkupBodyParser();
	
	// if content is unknown, use URL extension (if any) to guess type
	if (!theRepHdr.knownContentType() && OidImpliesMarkup(theOid, theRepContentCfg))
		return selectMarkupBodyParser();

	// parse if domestic content may have embedded tags
	if (!theOid.foreignUrl() && theRepContentCfg->hasEmbedCont())
		return selectMarkupBodyParser();

	// do not parse otherwise
	return AnyBodyParser::GetOne(this);
}

BodyParser *HttpCltXact::selectMarkupBodyParser() {
	if (!thePage) {
		thePage = PageInfo::Create();
		thePage->start = theStartTime;
	}
	if (theOwner->cfg()->followAllUris(theRepHdr))
			return UriScriptBodyParser::GetOne(this, theOwner->cfg());
	return MarkupBodyParser::GetOne(this, theOwner->cfg());
}

const ObjTimes &HttpCltXact::olcTimes() const {
	// not calculated or not configured, but can be
	if (theOlcTimes.lmt() < 0 && theRepContentCfg) 
		theRepContentCfg->calcTimes(theOid, theOlcTimes);
	return theOlcTimes;
}

void HttpCltXact::checkAcl() {
	const AclGroup &acl = theOwner->cfg()->acl();
	if (!acl) // no access controls configured
		return;

	if (!acl.needsCheck(theOid.foreignUrl()))
		return;

	if (RepHdr::PositiveStatusCode(theHttpStatus)) {
		if (!theOid.foreignUrl() && theOid.foreignSrc())
			checkAclMatch(acl.rewrite(), "rewritten");
		else
			checkAclMatch(acl.allow(), "allowed");
		return;
	} 

	if (theHttpStatus == RepHdr::sc407_ProxyAuthRequired) {
		// do not check acls if we did not authenticate yet but can
		if (!theCred.image() && theOwner->hasCredentials())
			return;
		// do not check acls if we sent invalid credentials
		if (theCred.image() && !theCred.valid())
			return;
	}

	checkAclMatch(acl.deny(), "denied");
}

void HttpCltXact::checkAclMatch(const RegExGroup *matchedGrp, const char *action) {
	RegExMatchee m;
	buildAclMatchee(m);
	static Array<RegExGroup*> matches;
	matches.reset();
	theOwner->cfg()->acl().match(m, matches);

	bool explainMatch = false;

	if (matches.count() == 0)
		explainMatch = ReportError(errAclNoMatches);
	else
	if (matches.count() == 1 && matches.last() != matchedGrp)
		explainMatch = ReportError(errAclWrongMatch);
	else
	if (matches.count() > 1)
		explainMatch = ReportError(errAclManyMatches);

	if (explainMatch)
		explainAclMatch(m, action, matches);
}

void HttpCltXact::buildAclMatchee(RegExMatchee &m) const {
	// form complete URL
	static WrBuf buf;
	buf.reset();

	ofixedstream os(buf.space(), buf.spaceSize()-Size(1));
	m.urlHost = buf.space() + 0; // URL host alone
	Oid2UrlHost(theOid, false, os);
	Should(os << ends);
	
	m.urlPath = buf.space() + os.tellp(); // URL path alone
	Oid2UrlPath(theOid, os);
	Should(os << ends);
	
	m.url = buf.space() + os.tellp(); // complete URL
	Oid2Url(theOid, os);
	Should(os << ends);

	buf.appended(Size(os.tellp()));
	buf.append("", 1); // terminate even if stream is full

	m.userName = theCred.image().cstr();
	m.memberships = &theOwner->memberships();
}

void HttpCltXact::explainAclMatch(const RegExMatchee &m, const char *action, const Array<RegExGroup*> &ourMatches) const {
	ostream &os = Comment(6) << "URL: " << m.url << endl;
	if (theCred.valid())
		os << "\tuser: " << theCred.image() << endl;
	os << "\tmembership maps: " << m.memberships->count() << endl;
	os << "\tgroups: ";
	for (int g = 0; g < m.memberships->count(); ++g) {
		if (g)
			os << "; ";
		dumpMatchingGroupNames(os, m.memberships->item(g));
		if (g >= 10) {
			os << "; ...";
			break;
		}
	}
	os << endl;
	os << "\twas probably " << action
		<< " but matches " << ourMatches.count() << " rule(s)";
	for (int i = 0; i < ourMatches.count(); ++i) {
		os << (i == 0 ? ": " : ", ");
		os << theOwner->cfg()->acl().ruleName(ourMatches[i]);
	}
	os << endc;
}

void HttpCltXact::dumpMatchingGroupNames(ostream &os, const MembershipMap *map) const {
	MembershipMap::GroupIterator i = map->groupIterator(theCred);
	for (int count = 0; i && count <= 5; ++count, ++i) {
		if (count)
			os << ", ";
		os << *i;
	}
}

void HttpCltXact::checkFreshness() {
	// cannot check without the date header
	if (theRepHdr.theDate < 0)
		return;

	// skip in-transit modicications to avoid false errors due to racing
	if (HttpDateAtMost(theStartTime) <= olcTimes().lmt())
		return;

	// response is stale if it was generated before modification time
	if (theRepHdr.theDate < olcTimes().lmt())
		noteError(errStaleHit);
}

bool HttpCltXact::cfgAbortedReply() const {
	Assert(theRepSize.expected().known());
	Size newRepSize = theRepSize.expected();

	if (!theRepHdr.theAbortCoord)
		return false;

	const Size abSz =
		theRepHdr.theAbortCoord.pos(theRepHdr.theHdrSize, theRepSize.expected()-theRepHdr.theHdrSize);

	if (abSz < 0)
		return false;

	/* expecting abort, sooner or later */

	newRepSize = abSz;

	// we may have leftovers if we were parsing aborted content
	if (abSz <= theRepSize.actual() + unconsumed())
		return true; // reached abort level

	return false;
}

Error HttpCltXact::setStatusCode(int aStatus) {
	theHttpStatus = aStatus;
	if (theHttpStatus != RepHdr::sc200_OK &&
		theHttpStatus != RepHdr::sc202_Accepted &&
		theHttpStatus != RepHdr::sc206_PartialContent &&
		theHttpStatus != RepHdr::sc304_NotModified &&
		theHttpStatus != RepHdr::sc401_Unauthorized &&
		theHttpStatus != RepHdr::sc403_Forbidden &&
		theHttpStatus != RepHdr::sc407_ProxyAuthRequired &&
		theHttpStatus != RepHdr::sc416_RequestedRangeNotSatisfiable &&
		theHttpStatus != RepHdr::sc417_ExpectationFailed &&
		!theRepHdr.redirect())
		return errHttpStatusCode;
	return Error();
}

void HttpCltXact::checkDateSync() {
	if (theRepHdr.theDate < 0) {
		ReportError(errHttpNoDate);
		return;
	}

	const Time maxGap = Time::Sec(60);

	// cannot measure drift using replies that took too long
	if (TheClock - theStartTime >= maxGap/2)
		return;

	if (theRepHdr.theDate > TheClock.time() + maxGap ||
		theRepHdr.theDate < TheClock.time() - maxGap) {
		if (ReportError(errSyncDate)) {
			HttpDatePrint(Comment << "request generated:  ", theStartTime) << endc;
			HttpDatePrint(Comment << "response generated: ", theRepHdr.theDate) << endc;
			HttpDatePrint(Comment << "response received:  ", TheClock.time()) << endc;
			const Time diff = theRepHdr.theDate - TheClock.time();
			Comment << "time difference:  " << diff << endc;
		}
		return;
	}
}

// called by BodyParsers, must not finish()
void HttpCltXact::noteContent(const ParseBuffer &content) {
	if (theRepHdr.theChecksum.set())
		theCheckAlg.update(content.data(), content.size());
}

// called by BodyParsers, must not finish()
Error HttpCltXact::noteEmbedded(ReqHdr &hdr) {
	TheEmbedStats.urlSeen++;

	const Error err = hdr.theUri.oid.foreignUrl() ?
		handleForeignEmbedOid(hdr) : handleEmbedOid(hdr);

	if (!err) {
		// "fix" new oid record
		hdr.theUri.oid.repeat(theOid.repeat());
		hdr.theUri.oid.hot(theOid.hot());
		hdr.theUri.oid.ims200(theOid.ims200());
		hdr.theUri.oid.ims304(theOid.ims304());
		hdr.theUri.oid.reload(theOid.reload());
		hdr.theUri.oid.get(true);
		theOwner->selectScheme(hdr.theUri.oid);
		theOwner->noteEmbedded(this, hdr.theUri.oid);
	}

	return err;
}

// called by BodyParsers, must not finish()
void HttpCltXact::noteTrailerHeader(const ParseBuffer &hdr) {
	if (ReportError(errTrailerHeader))
		printMsg(hdr.data(), hdr.size());
}

// called by BodyParsers, must not finish()
void HttpCltXact::noteEndOfTrailer() {
	Should(!theRepSize.expectingWhatParsed());
	theRepSize.expectingWhatParsed(true);
}

// called by BodyParsers, must not finish()
Error HttpCltXact::noteReplyPart(const RepHdr &hdr) {
	Error err;
	++theBodyPartCount;
	theBodyPartsSize += hdr.theContRangeLastByte - hdr.theContRangeFirstByte + Size(1);
	Assert(theRangesSize.known() && theRangeCount.known());
	if (theBodyPartsSize > theRangesSize ||
		theBodyPartCount > theRangeCount)
		err = errPartContBadCountOrSize;
	return err;
}

Error HttpCltXact::handleEmbedOid(ReqHdr &hdr) {
	Error err;
	if (validRelOid(hdr.theUri.oid)) {
		if (hdr.theUri.host)
			err = setViserv(hdr.theUri.host, hdr.theUri.oid);
		else
			hdr.theUri.oid.viserv(theOid.viserv()); // relative URL

		if (!err)
			theOwner->selectTarget(hdr.theUri.oid);
	} else {
		err = errBadEmbedUri;
	}

	return err;
}

Error HttpCltXact::handleForeignEmbedOid(ReqHdr &hdr) {
	Error err;
	//if (strncmp(hdr.theUri.pathBuf, "/cgi-bin/cntmgr.pl", 18) == 0)
	//	TheEmbedStats.scriptMgrUrlSeen++;
	
	if (!hdr.theUri.host) {
		// must set host name for Client to know where to send requests
		hdr.theUri.oid.viserv(theOid.viserv());
		char buf[4*1024];
		ofixedstream os(buf, sizeof(buf)-1);
		os << "http://";
		Oid2UrlHost(theOid, false, os);
		os << hdr.theUri.oid.foreignUrl() << ends;
		buf[sizeof(buf)-1] = '\0';
		hdr.theUri.oid.foreignUrl(buf);
	}

	//if (TheEmbedStats.scriptUrlSeen % 1000 == 0) {
	//	(clog << here << "tag url: ").write(hdr.theUri.pathBuf, hdr.theUri.pathLen);
	//	clog << endl;
	//}

	// hdr.theUri.oid is already set
	return err;
}

Error HttpCltXact::setViserv(const NetAddr &name, ObjId &oid) const {
	// XXX: merge with SrvXact::setViserv() ?
	int viserv = -1;
	if (!TheHostMap->find(name, viserv))
		return errForeignHostName;

	if (!theOid.foreignUrl() &&
		viserv != theOid.viserv())
		return errSrvRedirect; // disallow for now

	oid.viserv(viserv);
	return Error();
}

bool HttpCltXact::askedPeer() const {
	return thePeerState != peerUnknown;
}

bool HttpCltXact::usePeer() const {
	return thePeerState == peerSome;
}

void HttpCltXact::usePeer(bool doUse) {
	thePeerState = doUse ? peerSome : peerNone;
}

int HttpCltXact::cookiesSent() const {
	int n(0);
	if (theOwner->doCookies())
		if (const HttpCookies *cookies = theOwner->cookies(theOid))
			n = cookies->count();
	return n;
}

int HttpCltXact::cookiesRecv() const {
	return theRepHdr.theCookieCount;
}
