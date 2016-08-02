
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"
#include "xstd/Clock.h"
#include "xstd/PrefixIdentifier.h"
#include "xstd/gadgets.h"
#include "base/OLog.h"
#include "base/AddrParsers.h"
#include "base/polyLogCats.h"
#include "runtime/ErrorMgr.h"
#include "runtime/HostMap.h"
#include "runtime/HttpCookies.h"
#include "runtime/HttpDate.h"
#include "runtime/LogComment.h"
#include "runtime/httpText.h"
#include "runtime/httpHdrs.h"
#include "runtime/polyErrors.h"


/* internal type to store static parsing info */
class MsgHdrParsTab {
	public:
		MsgHdrParsTab();
		~MsgHdrParsTab();

	public:
		PrefixIdentifier *ids;
		Array<MsgHdr::Parser*> *parsers;
};


MsgHdrParsTab *ReqHdr::TheParsTab = 0;
MsgHdrParsTab *RepHdr::TheParsTab = 0;


/* HttpUri */
HttpUri::HttpUri(): pathBuf(0), pathLen(-1) {
	oid.scheme(Agent::pHTTP);
}


/* MsgHdr */

MsgHdr::MsgHdr(const MsgHdrParsTab &aTab): theParsTab(aTab),
	theCookies(0), createdCookies(false) {
	reset();
}

MsgHdr::~MsgHdr() {
	if (theCookies && createdCookies)
		delete theCookies;
}

void MsgHdr::reset() {
	theHdrSize = 0;
	theHttpVersion.reset();
	theContSize = -1;
	theChecksum.reset();
	theDate = Time();
	theGroupId.clear();
	theXactId.clear();
	theTarget = NetAddr();
	theRemWorld.reset();
	theAbortCoord.reset();
	thePhaseSyncPos = -1;
	theXactFlags = -1;
	theConnectionKeepAlive = kaDefault;
	theContType = ctUnknown;
	theTransferEncoding = tcNone;
	isCachable = true;
	theBoundary = String();
	theCookieCount = 0;
	if (theCookies) {
		if (createdCookies)
			delete theCookies;
		theCookies = 0;
	}
	needCookies = false;
	createdCookies = false;

	theBufBeg = theBufEnd = theSrchPtr = 0;
	theFields.reset();
	theSrchState = ssFirst;

	// do not reset parsing tables
}

bool MsgHdr::markupContent() const {
	return theContType == ctMarkup;	
}

bool MsgHdr::knownContentType() const {
	return theContType != ctUnknown;	
}

bool MsgHdr::multiRange() const {
	return theContType == ctMultiRange;
}

bool MsgHdr::chunkedEncoding() const {
	return theTransferEncoding == tcChunked;
}

bool MsgHdr::persistentConnection() const {
	if (theHttpVersion <= HttpVersion(1,0)) // 1.0: keep if explicitly told so
		return theConnectionKeepAlive == kaYes;
	else // 1.1: keep unless told otherwise
		return theConnectionKeepAlive != kaNo;
}

// note: buf does not have to be zero-terminated!
bool MsgHdr::parse(const char *buf, Size sz) {
	if (!theBufBeg) { // have not started the search yet
		theBufBeg = theSrchPtr = buf;
		Assert(theSrchState == ssFirst);
	} else {           // continue search
		Assert(theBufBeg == buf);
		Assert(theSrchState != ssFound);
	}

	theBufEnd = buf + sz; // to be refined later
	while (theSrchPtr < theBufEnd && theSrchState != ssFound) {
		// search for LF
		if (theSrchState == ssFirst) {
			do {
				if (*theSrchPtr++ == '\n') {
					theSrchState = ssSkip;
					break;
				}
			} while (theSrchPtr < theBufEnd);
		}

		// LF after skipping optional CRs means end-of-headers
		while (theSrchState == ssSkip && theSrchPtr < theBufEnd) {
			if (*theSrchPtr == '\n') {
				theSrchState = ssFound;
			} else
			if (*theSrchPtr != '\r') {
				theFields.append(theSrchPtr); // start of a header!
				theSrchState = ssFirst;
			}
			++theSrchPtr;
		}
	}

	if (theSrchState != ssFound)
		return false;

	// found end-of-headers!
	theBufEnd = theSrchPtr;
	theHdrSize = theBufEnd - theBufBeg;

	// now parse known fields
	// luckily, we already know field starts!
	parseFields();
	return true;
}

void MsgHdr::parseFields() {
	const char *eoh = theBufEnd;
	// skip end-of-headers CRLF
	while (theBufBeg < eoh && eoh[-1] == '\n') --eoh;
	while (theBufBeg < eoh && eoh[-1] == '\r') --eoh;

	parseRLine(theBufBeg, theFields.count() ? theFields[0] : eoh);

	for (int i = theFields.count()-1; i >= 0; --i) {
		const char *hdr = theFields[i];
		const int len = eoh-hdr; // approximate (includes crlfs)
		const int id = theParsTab.ids->lookup(hdr, len);
		if (id > 0) {
			const char *val = hdr + theParsTab.ids->string(id).len();
			while (isspace(*val)) ++val;
			Parser p = *theParsTab.parsers->item(id);
			(this->*p)(val, eoh);
		}
		eoh = hdr;
	}
}

bool MsgHdr::parseHttpVersion(const char *&beg, const char *end, HttpVersion &v) {
	const char *p = 0;
	int major = -1, minor = -1;
	if (isInt(beg, major, &p) && p+1 < end && *p == '.' && isInt(p+1, minor, &p)) {
		v = HttpVersion(major, minor);
		beg = p;
		return true;
	}
	return false;
}

bool MsgHdr::ParseHostInUri(const char *&start, const char *eorl, NetAddr &host) {
	if (const char *newStart = SkipHostInUri(start, eorl, host)) {
		start = newStart;
		return true;
	}
	return false;
}

bool MsgHdr::ParseSingleRange(const char *&buf, const char *eoh, Size &firstByte, Size &lastByte) {
	int i;
	const char *p;
	if (*buf == '-') {
		firstByte = -1;
		buf += 1;
	}
	else {
		if (!isInt(buf, i, &p) ||
			p >= eoh ||
			*p != '-')
			return false;
		firstByte = i;
		buf = p + 1;
	}
	i = -1;
	if (buf == eoh) {
		lastByte = -1;
		return (firstByte >= 0); // "a-" ranges
	}
	else
	if (isInt(buf, i, &p)) {
		buf = p;
		lastByte = i;
		return (firstByte <= lastByte); // "a-b" and "-b" ranges
	}
	return false;
}

// Find the first supported auth scheme and parse its value.
// The scheme name may be followed by authentication data.
bool MsgHdr::ParseAuthenticate(const char *&buf, const char *eoh, AuthChallenge &auth) {
	// Header fields are scanned in the reverse order of appearance so the
	// last header field we see and honor is the first one in the header.

	static PrefixIdentifier authSchemeIdentifier;
	if (!authSchemeIdentifier.count()) {
		authSchemeIdentifier.add("Basic", authBasic);
		authSchemeIdentifier.add("NTLM", authNtlm);
		authSchemeIdentifier.add("Negotiate", authNegotiate);
		authSchemeIdentifier.optimize();
	}

	// trim left
	while (buf < eoh && isspace((unsigned char) *buf))
		++buf;

	const int scheme = authSchemeIdentifier.lookup(buf, eoh-buf);

	if (!scheme) {
		static unsigned count(0);
		if (count++ < 10) {
			Comment(1) << "error: unsupported authentication scheme" << endl
				<< "header: ";
			Comment.write(buf, eoh-buf);
			Comment << endc;
		}
		return true; // not a parsing error
	}

	auth.scheme = HttpAuthScheme(scheme);

	// skip scheme name
	buf += authSchemeIdentifier.string(scheme).len();

	// trim spaces after the scheme name
	while (buf < eoh && isspace((unsigned char) *buf))
		++buf;

	// trim right
	while (buf < eoh && isspace((unsigned char) *(eoh-1)))
		--eoh;

	auth.params = String(buf, eoh-buf);
	return true;
}

bool MsgHdr::parseUri(const char *&buf, const char *end, HttpUri &uri) {
	const char *const start(buf);
	bool foreignHost(false);
	// see if there is a protocol://host prefix
	if (*start != '/') {
		ParseHostInUri(buf, end, uri.host);
		foreignHost = !TheHostMap->find(uri.host);
	}

	uri.pathBuf = buf; // includes leading '/'
	if (!uri.oid.parse(buf, end) ||
		foreignHost)
		uri.oid.foreignUrl(String(start, buf - start));
	uri.pathLen = buf - uri.pathBuf;
	return true;
}

bool MsgHdr::parseContLen(const char *buf, const char *) {
	theContSize = xatoi(buf, -1);
	return theContSize >= 0;
}

bool MsgHdr::parseContMd5(const char *buf, const char *eoh) {
	if (DecodeBase64(buf, eoh - buf, theChecksum.buf(), theChecksum.size()) == theChecksum.size()) {
		theChecksum.set(true);
		return true;
	}
	static unsigned count(0);
	if (count++ < 10) {
		Comment(1) << "error: malformed Content-MD5 header: ";
		Comment.write(buf, eoh-buf);
		Comment << endc;
	}
	return false;
}

bool MsgHdr::parseContType(const char *buf, const char *eoh) {
	theContType = ctOther; // default
	if (strncasecmp(buf, "text/", 5) == 0) {
		buf += 5;
		if (buf+4 <= eoh && strncasecmp(buf+2, "ml", 2) == 0)
			theContType = ctMarkup;
		else
		if (buf+4 <= eoh && strncasecmp(buf+1, "ml", 2) == 0)
			theContType = ctMarkup;
		else
		if (buf+3 <= eoh && strncasecmp(buf, "css", 3) == 0)
			theContType = ctMarkup;
	}
	else
	if (strncasecmp(buf, "multipart/byteranges; boundary=", 31) == 0) {
		buf += 31;
		// skip end-of-header CRLF
		if (buf < eoh && eoh[-1] == '\n')
			--eoh;
		if (buf < eoh && eoh[-1] == '\r')
			--eoh;
		if (buf < eoh) {
			theBoundary = String(buf, eoh - buf);
			theContType = ctMultiRange;
		}
	}

	return true;
}

bool MsgHdr::parseDate(const char *buf, const char *eoh) {
    theDate = HttpDateParse(buf, eoh - buf);
    return theDate >= 0;
}

bool MsgHdr::parsePragma(const char *buf, const char *) {
	if (!strncasecmp("no-cache", buf, 8))
		isCachable = false;
	else
		return false;
	return true;
}

bool MsgHdr::parseCControl(const char *buf, const char *) {
	if (!strncasecmp("no-cache", buf, 8)) {
		isCachable = false;
		return true;
	}
	return false;
}

bool MsgHdr::parseXXact(const char *buf, const char *eoh) {
	return
		theGroupId.parse(buf, eoh) && *buf == ' ' &&
		theXactId.parse(++buf, eoh) && *buf == ' ' &&
		isInt(++buf, theXactFlags, 0, 16);
}

bool MsgHdr::parseXRemWorld(const char *buf, const char *eoh) {
	return theRemWorld.parse(buf, eoh);
}

bool MsgHdr::parseXAbort(const char *buf, const char *) {
	RndGen::Seed whether = 0;
	RndGen::Seed where = 0;
	const char *p = 0;
	if (isInt64(buf, whether, &p) && *p == ' ' && isInt64(p+1, where)
		&& whether && where) {
		theAbortCoord.configure(whether, where);
		return true;
	}

	return false;
}

bool MsgHdr::parseXPhaseSyncPos(const char *buf, const char *) {
	thePhaseSyncPos = xatoi(buf, 0);
	return true;
}

bool MsgHdr::parseXTarget(const char *buf, const char *eoh) {
	// always expect explicit port
	return ParseNetAddr(buf, eoh, -1, theTarget);
}

/* XXX: Connection and other headers may have a _list_ of options */

bool MsgHdr::parseConnection(const char *buf, const char *) {
	if (!strncasecmp("close", buf, 5))
		theConnectionKeepAlive = kaNo;
	else
	if (!strncasecmp("keep", buf, 4))
		theConnectionKeepAlive = kaYes;
	else
		return false;
	return true;
}

bool MsgHdr::parseTransferEncoding(const char *buf, const char *) {
	if (!strncasecmp("chunked", buf, 7))
		theTransferEncoding = tcChunked;
	else
	if (!strncasecmp("identity", buf, 8))
		theTransferEncoding = tcIdentity;
	else
		theTransferEncoding = tcOther;
	return true;
}

bool MsgHdr::parseCookie(const char *buf, const char *eoh) {
	++theCookieCount;
	HttpCookie *cookie(0);
	if (needCookies) {
		cookie = HttpCookie::Parse(buf, eoh);
		if (Should(cookie)) {
			if (!theCookies) {
				theCookies = new HttpCookies;
				createdCookies = true;
			}
			theCookies->add(cookie);
		}
	}
	return cookie;
}

// adds definitions common to replies and requests
void MsgHdr::Configure(MsgHdrParsTab &tab) {
	AddParser(hfpDate, &MsgHdr::parseDate, tab);
	AddParser(hfpContLength, &MsgHdr::parseContLen, tab);
	AddParser(hfpContMd5, &MsgHdr::parseContMd5, tab);
	AddParser(hfpContType, &MsgHdr::parseContType, tab);
	AddParser(hfpCacheControl, &MsgHdr::parseCControl, tab);
	AddParser(hfpConnection, &MsgHdr::parseConnection, tab);
	AddParser(hfpPragma, &MsgHdr::parsePragma, tab);
	AddParser(hfpProxyConnection, &MsgHdr::parseConnection, tab);
	AddParser(hfpTransferEncoding, &MsgHdr::parseTransferEncoding, tab);
	AddParser(hfpXXact, &MsgHdr::parseXXact, tab);
	AddParser(hfpXRemWorld, &MsgHdr::parseXRemWorld, tab);
	AddParser(hfpXAbort, &MsgHdr::parseXAbort, tab);
	AddParser(hfpXPhaseSyncPos, &MsgHdr::parseXPhaseSyncPos, tab);
	AddParser(hfpXTarget, &MsgHdr::parseXTarget, tab);
}

int MsgHdr::AddParser(const String &field, Parser parser, MsgHdrParsTab &where) {
	Assert(field);
	// remove trailing space from "Header: " (but not from "METHOD ") fields
	const String trimmedField = isspace(field.last()) && field.chr(':') ?
		field(0, field.len()-1) : field;
	Assert(trimmedField);

	const int id = where.ids->add(trimmedField);
	where.parsers->put(new Parser(parser), id);
	return id;
}

void MsgHdr::store(OLog &log) const {
	log
		<< theHdrSize
		<< (int)theDate.sec()
		<< theContSize
		// << theChecksum
		<< theGroupId
		<< theXactId
		<< theTarget
		<< theHttpVersion.vMinor() // XXX: log major too
		<< (int)theConnectionKeepAlive
		<< isCachable
		// XXX: not stored or loaded: theTransferEncoding, theContType
		;
}

void MsgHdr::collectCookies(HttpCookies *cookies) {
	Assert(!needCookies);
	Assert(!theCookies);
	needCookies = true;
	theCookies = cookies;
}

// these should never be called
inline bool dontCallMe() { Assert(0); return false; }
bool MsgHdr::parseGetReqLine(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseHeadReqLine(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parsePostReqLine(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parsePutReqLine(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseHost(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseServer(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseProxyAuthenticate(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseWwwAuthenticate(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseLocation(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseLMT(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseExpires(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseIms(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseAcceptEncoding(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseXLocWorld(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseContRange(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseRange(const char *, const char *) { return dontCallMe(); }
bool MsgHdr::parseExpect(const char *, const char *) { return dontCallMe(); }


/* ReqHdr */

ReqHdr::ReqHdr(): MsgHdr(*TheParsTab), isHealthCheck(false),
	isAcceptingGzip(false) {
}

void ReqHdr::reset() {
	MsgHdr::reset();
	theUri = HttpUri();
	theIms = Time();
	theLocWorld.reset();
	isHealthCheck = false;
	isAcceptingGzip = false;
	theRanges.clear();
	expect100Continue = false;
}

bool ReqHdr::parseRLine(const char *buf, const char *eorl) {
	const int id = theParsTab.ids->lookup(buf, eorl - buf);
	if (id > 0) {
		buf += theParsTab.ids->string(id).len();
		while (isspace(*buf)) ++buf;

		Parser p = *theParsTab.parsers->item(id);
		return (this->*p)(buf, eorl);
	}
	return false;
}

bool ReqHdr::parseAnyReqLine(const char *buf, const char *eorl) {
	// a "well-known" health check uri
	static const String health = "/health";
	isHealthCheck = health.casePrefixOf(buf, eorl-buf);

	parseUri(buf, eorl, theUri);
	if (const char *proto = StrBoundChr(buf, ' ', eorl)) {
		// optimization: not checking for "HTTP/" match
		proto += 6;
		if (proto < eorl)
			parseHttpVersion(proto, eorl, theHttpVersion);
	}
	return true;
}

bool ReqHdr::parseGetReqLine(const char *buf, const char *eorl) {
	if (parseAnyReqLine(buf, eorl)) {
		theUri.oid.get(true);
		return true;
	}
	return false;
}

bool ReqHdr::parseHeadReqLine(const char *buf, const char *eorl) {
	if (parseAnyReqLine(buf, eorl)) {
		theUri.oid.head(true);
		return true;
	}
	return false;
}

bool ReqHdr::parsePostReqLine(const char *buf, const char *eorl) {
	if (parseAnyReqLine(buf, eorl)) {
		theUri.oid.post(true);
		return true;
	}
	return false;
}

bool ReqHdr::parsePutReqLine(const char *buf, const char *eorl) {
	if (parseAnyReqLine(buf, eorl)) {
		theUri.oid.put(true);
		return true;
	}
	return false;
}

bool ReqHdr::parseHost(const char *buf, const char *eoh) {
	// if there is no port set it to 0
	return ParseNetAddr(buf, eoh, 0, theUri.host);
}

bool ReqHdr::parseIms(const char *buf, const char *eoh) {
	theIms = HttpDateParse(buf, eoh - buf);
	return theIms >= 0;
}

bool ReqHdr::parseAcceptEncoding(const char *buf, const char *eoh) {
	// XXX: these checks ignore "q=0" preferences
	isAcceptingGzip = StrBoundChr(buf, '*', eoh) ||
		StrBoundStr(buf, "gzip", eoh); // XXX: codings are case-insensitive
	return true;
}

bool ReqHdr::parseXLocWorld(const char *buf, const char *eoh) {
	return theLocWorld.parse(buf, eoh);
}

bool ReqHdr::parseRange(const char *buf, const char *eoh) {
	bool res = true;

	// skip end-of-header CRLF
	if (buf < eoh && eoh[-1] == '\n')
		--eoh;
	if (buf < eoh && eoh[-1] == '\r')
		--eoh;

	while (buf < eoh) {
		ByteRange range;
		if (!ParseSingleRange(buf, eoh, range.theFirstByte, range.theLastByte)) {
			res = false;
			break;
		}
		if (buf < eoh &&
			*buf != ',') {
			res = false;
			break;
		}
		++buf;
		theRanges.push_back(range);
	}
	if (!res)
		theRanges.clear();
	return !theRanges.empty();
}

bool ReqHdr::parseExpect(const char *buf, const char *) {
	if (!strncasecmp("100-continue", buf, 12)) {
		expect100Continue = true;
		return true;
	}
	return false;
}

void ReqHdr::store(OLog &log) const {
	MsgHdr::store(log);
	log << theUri.host << theUri.oid << (int)theIms.sec();
}

bool ReqHdr::expectBody() const {
	return theUri.oid.post() || theUri.oid.put();
}

bool ReqHdr::acceptedEncoding(int coding) const {
	return coding == codingIdentity || // always acceptable for now
		(coding == codingGzip && isAcceptingGzip);
}

void ReqHdr::Configure() {
	TheParsTab = new MsgHdrParsTab();
	MsgHdr::Configure(*TheParsTab);
	AddParser(hfpHost, &MsgHdr::parseHost, *TheParsTab);
	AddParser(hfpIMS, &MsgHdr::parseIms, *TheParsTab);
	AddParser(hfpXLocWorld, &MsgHdr::parseXLocWorld, *TheParsTab);
	AddParser(hfpAcceptEncoding, &MsgHdr::parseAcceptEncoding, *TheParsTab);
	AddParser(hfpRange, &MsgHdr::parseRange, *TheParsTab);
	AddParser(hfpExpect, &MsgHdr::parseExpect, *TheParsTab);
	AddParser(hfpCookie, &MsgHdr::parseCookie, *TheParsTab);

	// request method parsers use the same index/interface as field parsers
	AddParser(rlpGet, &MsgHdr::parseGetReqLine, *TheParsTab);
	AddParser(rlpHead, &MsgHdr::parseHeadReqLine, *TheParsTab);
	AddParser(rlpPost, &MsgHdr::parsePostReqLine, *TheParsTab);
	AddParser(rlpPut, &MsgHdr::parsePutReqLine, *TheParsTab);
}

void ReqHdr::Clean() {
	delete TheParsTab;
	TheParsTab = 0;
}


/* RepHdr */

bool RepHdr::PositiveStatusCode(int code) {
     /* 1xx: Informational - Request received, continuing process
      * 2xx: Success - The action was successfully received,  understood, and accepted
      * 3xx: Redirection - Further action must be taken in order to complete the request
      * 4xx: Client Error - The request contains bad syntax or cannot be fulfilled
      * 5xx: Server Error - The server failed to fulfill an apparently valid request */
	return 100 <= code && code < 400;
}

RepHdr::RepHdr(): MsgHdr(*TheParsTab), theStatus(scUnknown) {
}

void RepHdr::reset() {
	MsgHdr::reset();
	theServer = String();
	theProxyAuthenticate = theOriginAuthenticate = AuthChallenge();
	theLocn = HttpUri();
	theLMT = theExpires = Time();
	theStatus = scUnknown;
	theContRangeFirstByte = -1;
	theContRangeLastByte = -1;
	theContRangeInstanceLength = -1;
}

bool RepHdr::expectPolyHeaders() const {
	return expectBody() &&
		theStatus != sc401_Unauthorized &&
		theStatus != sc407_ProxyAuthRequired &&
		theStatus != sc403_Forbidden &&
		!redirect();
}

// RFC 2616: All responses to the HEAD request method MUST NOT include
// a message-body. All 1xx , 204, and 304 responses MUST NOT include a
// message-body. All other responses do include a message-body.
bool RepHdr::expectBody() const {
	// note: we cannot handle the HEAD case here; the caller should
	if ((100 <= theStatus && theStatus < 200) ||
		theStatus == sc204_NoContent ||
		theStatus == sc304_NotModified)
		return false;
	return true;
}

bool RepHdr::redirect() const {
	return 
		theStatus == sc300_Choices ||
		theStatus == sc302_Found ||
		theStatus == sc303_Other ||
		theStatus == sc307_TmpRedir;
}

Time RepHdr::calcLmt() const {
	if (theLMT >= 0)
		return theLMT;
	if (theDate >= 0)
		return theDate;
	return TheClock;
}

bool RepHdr::parseRLine(const char *buf, const char *eorl) {
	if (strncasecmp("HTTP/", buf, 5) != 0)
		return false;
	buf += 5;
	if (buf >= eorl || !parseHttpVersion(buf, eorl, theHttpVersion))
		return false;
	buf += 1;
	return isInt(buf, theStatus);
}

bool RepHdr::parseServer(const char *buf, const char *eoh) {
	theServer = String(buf, eoh-buf);
	return theServer.len() > 0;
}

bool RepHdr::parseProxyAuthenticate(const char *buf, const char *eoh) {
	return ParseAuthenticate(buf, eoh, theProxyAuthenticate);
}

bool RepHdr::parseWwwAuthenticate(const char *buf, const char *eoh) {
	return ParseAuthenticate(buf, eoh, theOriginAuthenticate);
}

bool RepHdr::parseLocation(const char *buf, const char *eoh) {
	return parseUri(buf, eoh, theLocn);
}

bool RepHdr::parseLMT(const char *buf, const char *eoh) {
    theLMT = HttpDateParse(buf, eoh - buf);
    return theLMT >= 0;
}

bool RepHdr::parseExpires(const char *buf, const char *eoh) {
    theExpires = HttpDateParse(buf, eoh - buf);
    return theExpires >= 0;
}

bool RepHdr::parseContRange(const char *buf, const char *eoh) {
	if (strncasecmp(buf, "*/", 2) == 0)
		buf += 2;
	else
	if (!ParseSingleRange(buf, eoh, theContRangeFirstByte, theContRangeLastByte) ||
		theContRangeFirstByte < 0 ||
		theContRangeLastByte < 0 ||
		buf >= eoh ||
		*buf != '/')
		return false;
	buf += 1;
	int i = -1;
	if (*buf == '*' ||
		isInt(buf, i)) {
		theContRangeInstanceLength = i;
		return true;
	}
	return false;
}

void RepHdr::store(OLog &log) const {
	MsgHdr::store(log);
	log
		<< theStatus
		<< (int)theLMT.sec()
		<< (int)theExpires.sec()
		<< theContRangeFirstByte
		<< theContRangeLastByte
		<< theContRangeInstanceLength
		;
}

void RepHdr::Configure() {
	TheParsTab = new MsgHdrParsTab();
	MsgHdr::Configure(*TheParsTab);
	AddParser(hfpLocation, &MsgHdr::parseLocation, *TheParsTab);
	AddParser(hfpServer, &MsgHdr::parseServer, *TheParsTab);
	AddParser(hfpProxyAuthenticate, &MsgHdr::parseProxyAuthenticate, *TheParsTab);
	AddParser(hfpWwwAuthenticate, &MsgHdr::parseWwwAuthenticate, *TheParsTab);
	AddParser(hfpLmt, &MsgHdr::parseLMT, *TheParsTab);
	AddParser(hfpExpires, &MsgHdr::parseExpires, *TheParsTab);
	AddParser(hfpSetCookie, &MsgHdr::parseCookie, *TheParsTab);
	AddParser(hfpContRange, &MsgHdr::parseContRange, *TheParsTab);
}

void RepHdr::Clean() {
	delete TheParsTab;
	TheParsTab = 0;
}


/* MsgHdrParsTab */

MsgHdrParsTab::MsgHdrParsTab() {
	ids = new PrefixIdentifier;
	parsers = new Array<MsgHdr::Parser*>;
}

MsgHdrParsTab::~MsgHdrParsTab() {
	delete ids; ids = 0;
	while (parsers->count()) delete parsers->pop();
	delete parsers; parsers = 0;
}
