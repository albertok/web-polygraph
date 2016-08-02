
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/RndPermut.h"
#include "base/StatIntvlRec.h"
#include "csm/oid2Url.h"
#include "csm/BodyIter.h"
#include "csm/ContentMgr.h"
#include "runtime/globals.h"
#include "runtime/httpText.h"
#include "runtime/polyErrors.h"
#include "runtime/ErrorMgr.h"
#include "runtime/HostMap.h"
#include "runtime/HttpDate.h"
#include "runtime/HttpPrinter.h"
#include "runtime/LogComment.h"
#include "runtime/ObjUniverse.h"
#include "runtime/PopModel.h"
#include "runtime/SharedOpts.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/StatPhaseSync.h"
#include "server/Server.h"
#include "server/SrvCfg.h"

#include "server/HttpSrvXact.h"

HttpSrvXact::HttpSrvXact() {
	HttpSrvXact::reset();
}

void HttpSrvXact::reset() {
	SrvXact::reset();
	theReqHdr.reset();
	theHttpVersion.reset();
	theTimes.reset();
	theRanges.clear();
	the100ContinueState = csNone;
	theCookiesSentCount = 0;

	theRepFlags = 0;
}

void HttpSrvXact::doStart() {
	theHttpVersion = theOwner->httpVersion(); // may be downgraded later
	newState(stHdrWaiting);
	SrvXact::doStart();
}

void HttpSrvXact::logStats(OLog &ol) const {
	SrvXact::logStats(ol);
	ol << (int)theReqHdr.theIms.sec();
}

void HttpSrvXact::noteBodyDataReady() {
	consume(theReqSize.expectToGet(theConn->theRdBuf.contSize()));
	theConn->theRdBuf.pack();
	if (theReqSize.gotAll()) { // got everything expected
		theConn->theRd.stop(this);
		newState(stSpaceWaiting);
		if (theLastReqByteTime < 0)
			theLastReqByteTime = TheClock - theStartTime;
		if (the100ContinueState == csAllowed)
			the100ContinueState = csDone; // send no "100 Continue" if got all
	} else
	if (theConn->atEof()) { // premature eof
		if (theHttpStatus == RepHdr::sc417_ExpectationFailed) {
			finish(0); // not an error
			return;
		} else
		if (cfgAbortedReq()) {
			theOid.aborted(true);
			finish(0); // not an error, configuration told client to abort
			return;
		} else {
			finish(errPrematureEof);
			return;
		}
	} else // client probably wants to send more body data
	if (the100ContinueState == csAllowed || the100ContinueState == csDenied)
		newState(stSpaceWaiting); // but we need to respond with 100 or 417
}

void HttpSrvXact::noteBufReady() {
	// keep the write buffer full
	if (the100ContinueState != csAllowed && theBodyIter)
		theBodyIter->pour();
}

void HttpSrvXact::noteHdrDataReady() {
	if (theReqHdr.parse(theConn->theRdBuf.content(), theConn->theRdBuf.contSize())) {
		theReqSize.expect(theReqHdr.theHdrSize);
		if (const Error err = interpretHeader()) {
			finish(err);
			return;
		}
		Assert(the100ContinueState != csDone);
		newState(stBodyWaiting); // may become stSpaceWaiting to send 100/417
	} else
	if (theConn->theRdBuf.full()) { // header too big
		finish(errHugeHdr);
		return;
	} else
	if (theConn->atEof()) {         // premature end of headers
		finish(errPrematureEoh);
		return;
	}
}

void HttpSrvXact::noteRepSent() {
	if (the100ContinueState == csAllowed) {
		theContinueMsgTime = TheClock - theStartTime;
		the100ContinueState = csDone;
		theRepSize.reset();
		if (!theConn->theRd.theReserv)
			theConn->theRd.start(this);
		Assert(theState == stSpaceWaiting);
		newState(stBodyWaiting);
	} else
		finish(0);
}

// decide what kind of reply to build
void HttpSrvXact::makeRep(WrBuf &buf) {
	Assert(theRepContentCfg);

	const bool acceptableCoding = 
		theRepContentCfg->calcContentCoding(theOid, theReqHdr);

	// timestamps, cachability, aborts may be used for all types of replies
	theRepContentCfg->calcTimes(theOid, theTimes);

	theOid.cachable(theRepContentCfg->calcCachability(theOid));
	theOwner->cfg()->selectAbortCoord(theAbortCoord);
	theOwner->selectRepType(theOid);

	if (!theReqHdr.theRanges.empty()) {
		theOid.range(true);
		Assert(!theBodyIter);
		theBodyIter = theRepContentCfg->getBodyIter(theOid, &theRanges);
		normalizeRanges();
	}

	const char *repStart = buf.space();
	HttpPrinter hp(buf.space(), buf.spaceSize());

	if (the100ContinueState == csAllowed)
		make100Continue(hp);
	else
	if (the100ContinueState == csDenied)
		make417ExpectationFailed(hp);
	else
	if (!acceptableCoding)
		make406NotAcceptable(hp);
	else
	if (shouldMake302Found() && make302Found(hp))
		; // nothing to be done here
	else
	if (shouldMake304NotMod())
		make304NotMod(hp);
	else
	if (shouldMake416RequestedRangeNotSatisfiable())
		make416RequestedRangeNotSatisfiable(hp);
	else
		make2xxContent(hp);

	buf.appended(Size(hp.tellp()));

	Assert(theRepSize.header().known() && theRepSize.expected() >= theRepSize.header());

	theAbortSize = theAbortCoord.pos(theRepSize.header(), theRepSize.expected()-theRepSize.header());

	// dump reply header
	static int respCount = 0;
	if (!respCount++ || TheOpts.theDumpFlags(dumpRep, dumpHdr))
		printMsg(repStart, theRepSize.header());
}

Error HttpSrvXact::interpretHeader() {
	// do this before we start overwriting or dumping the URL/oid
	theOid = theReqHdr.theUri.oid;
	theOid.secure(theConn->sslActive());

	// set default port for Host: header
	if (!theReqHdr.theUri.host.port())
		theReqHdr.theUri.host.port(theOid.secure() ? 443 : 80);

	// XXX: pxy server cannot support ignoreUrls; see Server::hostIdx
	const bool ignoreUrls = grokUrl(theReqHdr.isHealthCheck);

	// XXX: report healthchecks once in a while and collect stats
	if (theReqHdr.isHealthCheck) {
		static bool didOnce = false;
		if (!didOnce) {
			Comment(6) << "fyi: first health check received from " << theConn->raddr() << ':' << endc;
			printMsg(theConn->theRdBuf, theReqHdr.theHdrSize);
			didOnce = true;
		}
	}

	// dump request header
	static int reqCount = 0;
	if (!reqCount++ || TheOpts.theDumpFlags(dumpReq, dumpHdr))
		printMsg(theConn->theRdBuf, theReqHdr.theHdrSize);

	if (theOid.foreignUrl())
		return errForeignUrl;

	// downgrade HTTP version if needed
	if (theReqHdr.theHttpVersion < theHttpVersion)
		theHttpVersion = theReqHdr.theHttpVersion;

	// calculate request size
	if (theReqHdr.expectBody()) {
		theReqOid.type(TheUnknownContentId);
		theReqSize.expectedBody(true);
		if (theReqHdr.theContSize < 0)
			return errReqBodyButNoCLen;
		theReqSize.expectMore(theReqHdr.theContSize);
	} else {
		theReqOid.type(TheBodilessContentId);
		if (!(theOid.get() && theReqHdr.theContSize == 0) &&
			theReqHdr.theContSize >= 0)
			return errUnexpectedCLen;
	}

	if (!ignoreUrls) {
		const NetAddr &host = theReqHdr.theUri.host;
		if (!host.knownAddr())
			return errNoHostName;

		if (const Error err = setViserv(host))
			return err; // setViserv called finish

		if (theOid.target() < 0) {
			// currently, servers do not use target but we complain
			// do identify prefetch requests and such
			if (!theReqHdr.theTarget && ReportError(errNoTarget) &&
				TheOpts.theDumpFlags(dumpErr, dumpAny))
					printMsg(theConn->theRdBuf, theReqHdr.theHdrSize);
			if (const Error err = setTarget(theReqHdr.theTarget))
				return err; // setTarget called finish
			Assert(theOid.target() >= 0);
		}

		if (const Error err = checkUri()) // must be done before we consume()
			return err;
	}

	// note: clt and srv code assume we consume only after parse!
	consume(theReqHdr.theHdrSize);
	theConn->theRdBuf.pack();

	// update history for this server
	if (theOid.viserv() >= 0 && theReqHdr.theLocWorld)
		updateUniverse(theReqHdr.theLocWorld);

	doPhaseSync(theReqHdr);

	// note if the client will close the connection
	theConn->lastUse(!theReqHdr.persistentConnection());

	theOid.reload(!theReqHdr.isCachable);

	// get content specs for this reply
	Assert(theOid.type() >= ContType::NormalContentStart());
	theRepContentCfg = TheContentMgr.get(theOid.type());

	if (theReqHdr.expect100Continue) {
		static RndGen rng;
		Assert(the100ContinueState == csNone);
		if (theOwner->cfg()->theReqBodyAllowed < 0 ||
			rng.event(theOwner->cfg()->theReqBodyAllowed))
			the100ContinueState = csAllowed;
		else
			the100ContinueState = csDenied;
	}

	theReqFlags = theReqHdr.theXactFlags;

	Assert(!theRepSize.header().known());

	return 0;
}

// merge with CltXact::cfgAbortedReply()?
bool HttpSrvXact::cfgAbortedReq() const {
	if (!theReqHdr.theAbortCoord)
		return false;

	const Size abSz = theReqHdr.theAbortCoord.pos(theReqHdr.theHdrSize, theReqSize.expected()-theReqHdr.theHdrSize);
	if (!Should(abSz >= 0))
		return false;

	if (abSz > theReqSize.actual())
		return false;

	return true;
}

// check that requested URI belongs to our server
Error HttpSrvXact::checkUri() {
	const int pathLen = theReqHdr.theUri.pathLen;
	const bool sane = Should(pathLen >= 0) &&
		Should(pathLen <= theConn->theRdBuf.contSize()) &&
		Should(theReqHdr.theUri.pathBuf);

	if (!sane) { // no point in high-level checks
		Comment << "internal error: inconsistent request, salvaged" << endc;
		return errOther;
	}

	// first check if the content type belongs to us
	if (!theOwner->cfg()->hasContType(theOid.type())) {
		if (ReportError(errMisdirRequest)) {
			Comment << "host " << theOwner->host()
				<< " does not have content type implied by request URI: ";
			Comment.write(theReqHdr.theUri.pathBuf, pathLen);
			Comment << endc;
		}
		return errOther;
	}

	// we cannot compare Oid2UrlPath() with pathBuf of special URLs
	if (pathLen >= 4 && strncmp("/pg/", theReqHdr.theUri.pathBuf, 4) == 0)
		return 0;

	static char buf[16*1024];
	ofixedstream os(buf, sizeof(buf)/sizeof(*buf));
	Oid2UrlPath(theOid, os);
	os << ends;
	buf[sizeof(buf)/sizeof(*buf) - 1] = '\0';

	const bool res = (pathLen+1 == (int)os.tellp()) &&
		strncmp(buf, theReqHdr.theUri.pathBuf, pathLen) == 0;

	if (!res) {
		if (ReportError(errMisdirRequest)) {
			Comment << "host: " << theOwner->host() << endc;
			(Comment << "received: ").write(theReqHdr.theUri.pathBuf, pathLen);
			Comment << endc;
			Oid2UrlPath(theOid, Comment << "expected: ");
			Comment << endc;
		}
		return errOther;
	}
	return 0;
}

Error HttpSrvXact::setViserv(const NetAddr &name) {
	int viserv = -1;
	HostCfg *host = TheHostMap->find(name, viserv);
	if (!host) {
		host = TheHostMap->find(theOwner->host(), viserv);
		const bool salvaged = Should(host != 0);
		if (ReportError(errForeignHostName)) {
			Comment << theOwner->host() << " server received request for " <<
				name << " which is not an address of any visible server, " <<
				(salvaged ? "salvaged" : "aborting transaction") <<
				endc;
		}
		if (!salvaged)
			return errForeignHostName;
	}

	if (!host->theUniverse)
		ObjUniverse::Add(host, new ObjUniverse());

	theOid.viserv(viserv);
	return 0;
}

Error HttpSrvXact::setTarget(const NetAddr &) {
	// XXX: the check below should know about DNS RR, etc.
	//if (target != theOwner->host())
	//	ReportError(errMisdirRequest);

	theOid.target(theOwner->hostIdx());
	return 0;
}

// take all ranges from theReqHdr and do "normalization"
// That is:
// 1. Remove unsatisfiable ranges.
// 2. Convert to a-b form where 0 <= a <= b <= full-entity-size - 1
//
// result is stored in theRanges member
void HttpSrvXact::normalizeRanges() {
	Assert(theRanges.empty());
	const Size contSize = theBodyIter->fullEntitySize();

	for (RangeList::const_iterator i = theReqHdr.theRanges.begin();
		i != theReqHdr.theRanges.end();
		++i) {
		if (i->theFirstByte >= contSize ||
			(i->theFirstByte < 0 &&
			i->theLastByte == 0))
			// Unsatisfiable range
			continue;

		ByteRange range;
		if (i->theFirstByte >= 0) {
			range.theFirstByte = i->theFirstByte;
			if (i->theLastByte < 0 ||
				i->theLastByte >= contSize)
				range.theLastByte = contSize - Size(1);
			else
				range.theLastByte = i->theLastByte;
		} else {
			// Suffix range
			if (i->theLastByte < contSize)
				range.theFirstByte = contSize - i->theLastByte;
			else
				range.theFirstByte = 0;
			range.theLastByte = contSize - Size(1);
		}
		theRanges.push_back(range);
	}
}

void HttpSrvXact::make100Continue(HttpPrinter &hp) {
	putResponseLine(hp, rls100Continue);
	hp << crlf; // end-of-headers

	theRepSize.header(Size(hp.tellp()));
	theRepSize.expect(theRepSize.header());
}

// build headers for a happy "200 OK" or "206 Partial Content" reply
void HttpSrvXact::make2xxContent(HttpPrinter &hp) {
	if (theBodyIter == 0)
		theBodyIter = theRepContentCfg->getBodyIter(theOid, &theRanges);

	if (!theOid.range()) {
		theHttpStatus = RepHdr::sc200_OK;
		putResponseLine(hp, rls200Ok);
	} else {
		theHttpStatus = RepHdr::sc206_PartialContent;
		putResponseLine(hp, rls206PartialContent);
	}

	put2xxContentHead(hp);
	hp << crlf; // end-of-headers

	theRepSize.header(Size(hp.tellp()));
	if (theOid.head()) {
		// do not send message body for HEAD
		theRepSize.expect(theRepSize.header());
		theBodyIter->putBack();
		theBodyIter = 0;
		theOid.type(TheBodilessContentId);
	}
	else {
		theRepSize.expectedBody(true);
		theRepSize.expect(theRepSize.header() + theBodyIter->contentSize());
		theBodyIter->start(&theConn->theWrBuf);
	}

	// note: non-200 and non-304 responses to IMS requests not covered yet
	theOid.ims200(theReqHdr.theIms >= 0);
}


bool HttpSrvXact::canMake302Found(ObjId &oid) const {
	Assert(theOwner->popModel());

	// at this time redirect to the same server only
	if (theOid.viserv() < 0)
		return false;

	// misconfiguration
	if (!theOwner->popModel())
		return false;

	ObjUniverse *const universe =
		TheHostMap->findUniverseAt(theOid.viserv());

	if (!universe || !universe->canRepeat(theOid.type()))
		return false;

	oid = theOid; // set viserv and such
	oid.name(-1);
	oid.type(-1); // overwrite name and type
	universe->repeat(oid, theOwner->popModel());
	return true;
}

bool HttpSrvXact::make302Found(HttpPrinter &hp) {
	if (theBodyIter) {
		theBodyIter->putBack();
		theBodyIter = 0;
	}

	ObjId newOid;

	// make sure we have an object to redirect to
	if (!canMake302Found(newOid)) {
		ReportError(errMake302Found);
		return false;
	}

	theHttpStatus = RepHdr::sc302_Found;
	putResponseLine(hp, rls302Found);

	hp.putHeaders(theOwner->cfg()->httpHeaders());
	putStdFields(hp);

	String url;
	{
		const int buf_size = 4*1024;
		static char buf[buf_size];
		ofixedstream url_os(buf, buf_size);
		Oid2Url(newOid, url_os) << ends;
		buf[buf_size-1] = '\0';
		url = buf;
	}
	if (hp.putHeader(hfpLocation))
		hp << url << crlf;

	const int clen = url.len() + text302Found.len();
	if (hp.putHeader(hfpContLength))
		hp << clen << crlf;

	putXFields(hp);

	hp << crlf; // end-of-headers

	// we determine the type of IMS request later
	theRepSize.header(Size(hp.tellp()));

	// text for humans
	Oid2Url(newOid, hp << text302Found);

	theRepSize.expect(theRepSize.header() + Size(clen));
	return true;
}

bool HttpSrvXact::shouldMake302Found() const {
	return theOid.repToRedir();
}

void HttpSrvXact::make304NotMod(HttpPrinter &hp) {
	openSimpleMessage(hp, RepHdr::sc304_NotModified, rls304NotModified, 0);
	closeSimpleMessage(hp, 0);
	theOid.ims304(true);
}

bool HttpSrvXact::shouldMake304NotMod() const {
	return 
		theReqHdr.theIms >= 0 && theTimes.lmt() >= 0 && 
		theTimes.lmt() <= theReqHdr.theIms;
}

void HttpSrvXact::make406NotAcceptable(HttpPrinter &hp) {
	ReportError(errNoAcceptableContentCoding);

	const String &body = text406NotAcceptable;
	openSimpleMessage(hp, RepHdr::sc406_NotAcceptable, rls406NotAcceptable, &body);

	if (theRepContentCfg->multipleContentCodings())
		hp.putHeader(hfVaryAcceptEncoding);

	closeSimpleMessage(hp, &body);
}

void HttpSrvXact::make416RequestedRangeNotSatisfiable(HttpPrinter &hp) {
	Assert(theBodyIter);

	const String &body = text416RequestedRangeNotSatisfiable;
	openSimpleMessage(hp, RepHdr::sc416_RequestedRangeNotSatisfiable, rls416RequestedRangeNotSatisfiable, &body);

	if (hp.putHeader(hfpContRange))
		hp << "*/" << (int)theBodyIter->fullEntitySize() << crlf;

	closeSimpleMessage(hp, &body);
}

bool HttpSrvXact::shouldMake416RequestedRangeNotSatisfiable() const {
	return (theOid.range() && theRanges.empty());
}

void HttpSrvXact::make417ExpectationFailed(HttpPrinter &hp) {
	const String &body = text417ExpectationFailed;
	openSimpleMessage(hp, RepHdr::sc417_ExpectationFailed, rls417ExpectationFailed, &body);
	closeSimpleMessage(hp, &body);
}

void HttpSrvXact::putResponseLine(ostream &os, const String &suffix) {
	if (theHttpVersion <= HttpVersion(1,0))
		os << protoHttp1p0;
	else
		os << protoHttp1p1;
	os << suffix;
}

// put well-known fields acceptable for both 304 and 200 replies
void HttpSrvXact::putStdFields(HttpPrinter &hp) const {
	// general-header fields

	if (hp.putHeader(hfpDate))
		HttpDatePrint(hp) << crlf;

	// persistency indication depends on HTTP version
	if (theHttpVersion <= HttpVersion(1,0)) {
		if (theConn->reusable())
			hp.putHeader(hfConnAliveOrg);
	} else {
		if (!theConn->reusable())
			hp.putHeader(hfConnCloseOrg);
	}

	// response-header fields (none)
	// entity-header fields

	if (theOid.cachable() &&
		theTimes.knownExp() &&
		hp.putHeader(hfpExpires))
		HttpDatePrint(hp, theTimes.exp()) << crlf;
}

// put extension header fields acceptable for both 302 and 200 replies
void HttpSrvXact::putXFields(HttpPrinter &hp) const {
	// put group ids with source/target ids on one line?
	if (hp.putHeader(hfpXTarget))
		hp << theOwner->host() << crlf;

	if (theReqHdr.theXactId &&
		hp.putHeader(hfpXXact)) {
		hp << TheGroupId
			<< ' ' << theReqHdr.theXactId.genMutant()
			<< ' ' << hex << theRepFlags << dec
			<< crlf;
	}

	if (theOid.viserv() >= 0 && theReqHdr.theRemWorld)
		putRemWorld(hp, theReqHdr.theRemWorld);

	if (hp.putHeader(hfpXAbort)) {
		hp << theAbortCoord.whether()
		<< ' ' << theAbortCoord.where()	<< crlf;
	}

	putPhaseSyncPos(hp, TheStatPhaseSync.phaseSyncPos());
}

void HttpSrvXact::put2xxContentHead(HttpPrinter &hp) {
	/* user-configured headers */

	if (theRepContentCfg)
		hp.putHeaders(theRepContentCfg->mimeHeaders());
	hp.putHeaders(theOwner->cfg()->httpHeaders());

	/* general-header fields */

	if (theOid.cachable())
		hp.putHeader(hfCcCachable);
	else {
		hp.putHeader(hfCcUncachable);
		hp.putHeader(hfPragmaUncachable);
	}

	putStdFields(hp);

	/* other entity-header fields */

	if (theOid.cachable() &&
		theTimes.showLmt() &&
		hp.putHeader(hfpLmt))
		HttpDatePrint(hp, theTimes.lmt()) << crlf;

	if (theRepContentCfg->multipleContentCodings())
		hp.putHeader(hfVaryAcceptEncoding);

	// Servers probably MUST NOT send a Content-MD5 header with
	// 206 responses. See
	// http://trac.tools.ietf.org/wg/httpbis/trac/ticket/178
	if (!theOid.range() && theRepContentCfg->calcChecksumNeed(theOid))
		putChecksum(*theRepContentCfg, theOid, hp);

	if (theOwner->isCookieSender)
		putCookies(hp);

	if (theBodyIter)
		theBodyIter->putHeaders(hp);

	/* extention-header fields (they are entity-header fields as well) */
	putXFields(hp);
}

void HttpSrvXact::putRemWorld(HttpPrinter &hp, const ObjWorld &oldWorld) const {
	ObjUniverse &universe = *TheHostMap->findUniverseAt(theOid.viserv());

	int sliceIdx;
	if (universe.find(oldWorld.id(), sliceIdx)) {
		const ObjWorld *const world = universe.newerWorld(oldWorld, sliceIdx);
		if (world && hp.putHeader(hfpXRemWorld))
			hp << *world << crlf;
	} 

	// else client-side sent remote world ID before the same public world ID
}

void HttpSrvXact::putCookies(ostream &os) {
	SrvCfg *cfg = theOwner->cfg();
	const int oidSeed = theOid.hash();

	// check wether we should send (set) cookies with this response
	const int seedSend = GlbPermut(oidSeed, rndCookieSend);
	RndGen rng(seedSend);
	if (!rng.event(cfg->theCookieSendProb))
		return;

	// calculate how many cookies we should send
	RndDistr *distrCount = seedOidDistr(cfg->theCookieCounts, rndCookieCount);
	theCookiesSentCount = (int)MiniMax(1.0, distrCount->trial(), (double)INT_MAX);
	
	RndDistr *sizeDistr = seedOidDistr(cfg->theCookieSizes, rndCookieSize);
	Size accSize = 0;
	for (int i = 0; i < theCookiesSentCount; ++i) {
		static const String cookieValuePfx = "sess";
		static const String cookieValueSfx = "\"";

		os << hfpSetCookie << cookieValuePfx << i << "=\"";

		WrBuf &buf = theConn->theWrBuf;
		const Size cookieSize = (int)MiniMax(0.0, sizeDistr->trial(), (double)INT_MAX);
		const Size cookieContentOff = IOBuf::RandomOffset(oidSeed, accSize);
		const Size usedSize = (streamoff)os.tellp();
		const Size spaceRemaining = buf.spaceSize() - usedSize;
		const bool fit = cookieSize + Size(cookieValueSfx.len() + 2) <= 
			spaceRemaining;

		if (!fit && ReportError(errCookiesDontFit)) {
			Comment << "cookie size: " << cookieSize << 
				" cookies buffered: " << i << '/' <<
				theCookiesSentCount << " or " <<
				accSize << "; space left: " << spaceRemaining << endc;
		}

		// yes, make buffer (ostream) full
		IOBuf::RandomFill(os, cookieContentOff, cookieSize, RndText());
		accSize += cookieSize;

		os << cookieValueSfx << crlf;

		if (!fit)
			break;
	}
}

void HttpSrvXact::openSimpleMessage(HttpPrinter &hp, const int status, const String &header, const String *const body) {
	theHttpStatus = status;
	putResponseLine(hp, header);

	hp.putHeaders(theOwner->cfg()->httpHeaders());
	putStdFields(hp);

	if (body && hp.putHeader(hfpContLength))
		hp << body->len() << crlf;

	putXFields(hp);
}

void HttpSrvXact::closeSimpleMessage(ostream &os, const String *const body) {
	if (theBodyIter) {
		theBodyIter->putBack();
		theBodyIter = 0;
	}

	os << crlf; // end-of-headers

	theRepSize.header(Size(os.tellp()));

	if (!theOid.head() && body) {
		// text for humans
		os << *body;
		theRepSize.expect(theRepSize.header() + Size(body->len()));
		theOid.type(TheUnknownContentId);
	} else {
		theRepSize.expect(theRepSize.header());
		theOid.type(TheBodilessContentId);
	}
}

void HttpSrvXact::doPhaseSync(const MsgHdr &hdr) const {
	Xaction::doPhaseSync(hdr);
	if (TheStatPhaseMgr.phaseSyncPos() < TheStatPhaseSync.phaseSyncPosMin()) {
		TheStatPhaseMgr->reachedPositiveGoal("all clients reached "
			"their phase goals");
	}
}

int HttpSrvXact::cookiesSent() const {
	return theCookiesSentCount;
}

int HttpSrvXact::cookiesRecv() const {
	return theReqHdr.theCookieCount;
}
