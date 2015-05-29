
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_HTTPHDRS_H
#define POLYGRAPH__RUNTIME_HTTPHDRS_H

#include <list>

#include "xstd/h/iostream.h"

#include "xstd/Array.h"
#include "xstd/NetAddr.h"
#include "xstd/Checksum.h"
#include "base/AuthPhaseStat.h"
#include "base/ObjId.h"
#include "runtime/Agent.h"
#include "runtime/ObjWorld.h"
#include "runtime/XactAbortCoord.h"
#include "runtime/HttpVersion.h"


class MsgHdrParsTab;
class HttpCookies;

// XXX: make HTTP-independent
enum HttpAuthScheme { authNone = AuthPhaseStat::sNone,
	authBasic = AuthPhaseStat::sBasic,
	authNtlm = AuthPhaseStat::sNtlm,
	authNegotiate = AuthPhaseStat::sNegotiate,
	authFtp = AuthPhaseStat::sFtp };

class HttpUri {
	public:
		HttpUri();

	public:
		ObjId oid;
		NetAddr host;
		const char *pathBuf;
		int pathLen;
};

struct ByteRange {
	Size theFirstByte;
	Size theLastByte;
};

// Encapsulates RFC 2617 "challenge" information
// Used to store Proxy-Authenticate and WWW-Authenticate header values
class AuthChallenge {
	public:
		AuthChallenge(): scheme(authNone) {}

		HttpAuthScheme scheme; // auth-scheme in RFC 2616
		String params; // 1#auth-param in RFC 2616
};

typedef std::list<ByteRange> RangeList;

// common interface for parsing HTTP requests and responses
class MsgHdr {
	public:
		typedef bool (MsgHdr::*Parser)(const char *buf, const char *eoh);

	protected:
		static bool ParseHostInUri(const char *&start, const char *eorl, NetAddr &host);
		static bool ParseSingleRange(const char *&buf, const char *eoh, Size &firstByte, Size &lastByte);
		static bool ParseAuthenticate(const char *&buf, const char *eoh, AuthChallenge &auth);

	public:
		MsgHdr(const MsgHdrParsTab &aTab);
		virtual ~MsgHdr();

		virtual void reset();

		bool persistentConnection() const;
		bool knownContentType() const;
		bool markupContent() const;
		bool multiRange() const;
		bool chunkedEncoding() const;

		bool parse(const char *buf, Size sz);
		bool parseUri(const char *&buf, const char *eoh, HttpUri &uri);

		virtual void store(OLog &log) const;

		void collectCookies(HttpCookies *cookies);

	protected:
		static void Configure(MsgHdrParsTab &tab);
		static int AddParser(const String &field, Parser parser, MsgHdrParsTab &tab);

	protected:
		void parseFields();

		virtual bool parseRLine(const char *buf, const char *eol) = 0;

	public:
		// can be used by both requests and replies
		bool parseHttpVersion(const char *&beg, const char *end, HttpVersion &v);
		bool parseDate(const char *buf, const char *eoh);
		bool parseContLen(const char *buf, const char *eoh);
		bool parseContMd5(const char *buf, const char *eoh);
		bool parseContType(const char *buf, const char *eoh);
		bool parsePragma(const char *buf, const char *eoh);
		bool parseCControl(const char *buf, const char *eoh);
		bool parseXXact(const char *buf, const char *eoh);
		bool parseXTarget(const char *buf, const char *eoh);
		bool parseXRemWorld(const char *buf, const char *eoh);
		bool parseXAbort(const char *buf, const char *eoh);
		bool parseXPhaseSyncPos(const char *buf, const char *eoh);
		bool parseConnection(const char *buf, const char *eoh);
		bool parseTransferEncoding(const char *buf, const char *eoh);
		bool parseCookie(const char *buf, const char *eoh);

		// these have to be here so we can register them in req
		virtual bool parseGetReqLine(const char *buf, const char *eorl);
		virtual bool parseHeadReqLine(const char *buf, const char *eorl);
		virtual bool parsePostReqLine(const char *buf, const char *eorl);
		virtual bool parsePutReqLine(const char *buf, const char *eorl);
		virtual bool parseHost(const char *buf, const char *eoh);
		virtual bool parseIms(const char *buf, const char *eoh);
		virtual bool parseXLocWorld(const char *buf, const char *eoh);
		virtual bool parseAcceptEncoding(const char *buf, const char *eoh);
		virtual bool parseExpect(const char *buf, const char *eoh);

		// these have to be here so we can register them in rep
		virtual bool parseServer(const char *buf, const char *eoh);
		virtual bool parseProxyAuthenticate(const char *buf, const char *eoh);
		virtual bool parseWwwAuthenticate(const char *buf, const char *eoh);
		virtual bool parseLocation(const char *buf, const char *eoh);
		virtual bool parseLMT(const char *buf, const char *eoh);
		virtual bool parseExpires(const char *buf, const char *eoh);
		virtual bool parseContRange(const char *buf, const char *eoh);
		virtual bool parseRange(const char *buf, const char *eoh);

	public:
		const MsgHdrParsTab &theParsTab;

		Size theHdrSize;
		HttpVersion theHttpVersion;
		Time theDate;          // Date: header field
		Size theContSize;
		xstd::Checksum theChecksum; // Content-MD5

		UniqId theGroupId;
		UniqId theXactId;
		NetAddr theTarget;

		ObjWorld theRemWorld; // remote object world specs
		XactAbortCoord theAbortCoord; // abort coordinates
		int thePhaseSyncPos; // index of a ready-to-stop phase

		int theXactFlags;

		enum { kaNo, kaDefault, kaYes } theConnectionKeepAlive;
		enum { ctUnknown, ctMarkup, ctOther, ctMultiRange } theContType; // content type
		enum { tcNone, tcChunked, tcIdentity, tcOther } theTransferEncoding;
		bool isCachable;
		String theBoundary;

		int theCookieCount; // number of cookies
		HttpCookies *theCookies; // a place to collect cookies, if needed

	protected:
		Array<const char *> theFields; // positions of header fields
		const char *theBufBeg, *theBufEnd;
		const char *theSrchPtr;        // parse buffer iterator
		enum { ssFirst, ssSkip, ssFound } theSrchState;
		bool needCookies; // whether cookies should be collected
		bool createdCookies; // true cookies were created during parsing
};

class ReqHdr: public MsgHdr {
	public:
		static void Configure();
		static void Clean();
		
	public:
		ReqHdr();

		virtual void reset();

		virtual void store(OLog &log) const;

		bool expectBody() const;
		bool acceptedEncoding(int coding) const;

	protected:
		virtual bool parseRLine(const char *buf, const char *eol);

		bool parseAnyReqLine(const char *buf, const char *eorl);

		virtual bool parseAcceptEncoding(const char *buf, const char *eoh);
		virtual bool parseGetReqLine(const char *buf, const char *eorl);
		virtual bool parseHeadReqLine(const char *buf, const char *eorl);
		virtual bool parsePostReqLine(const char *buf, const char *eorl);
		virtual bool parsePutReqLine(const char *buf, const char *eorl);
		virtual bool parseHost(const char *buf, const char *eoh);
		virtual bool parseIms(const char *buf, const char *eoh);
		virtual bool parseXLocWorld(const char *buf, const char *eoh);
		virtual bool parseRange(const char *buf, const char *eoh);
		virtual bool parseExpect(const char *buf, const char *eoh);

	protected:
		static MsgHdrParsTab *TheParsTab;

	public:
		HttpUri theUri;
		Time theIms;          // If-Modified-Since timestamp
		ObjWorld theLocWorld; // local world info
		bool isHealthCheck;
		bool isAcceptingGzip;

		RangeList theRanges;
		bool expect100Continue;
};

class RepHdr: public MsgHdr {
	public:
		enum StatusCode { scUnknown = -1, sc100_Continue = 100,
			sc200_OK = 200, sc202_Accepted = 202,
			sc204_NoContent = 204, sc206_PartialContent = 206,
			sc300_Choices = 300, sc302_Found = 302, sc303_Other = 303,
			sc304_NotModified = 304, sc307_TmpRedir = 307,
			sc401_Unauthorized = 401,
			sc403_Forbidden = 403,
			sc406_NotAcceptable = 406,
			sc407_ProxyAuthRequired = 407,
			sc416_RequestedRangeNotSatisfiable = 416,
			sc417_ExpectationFailed = 417 };

		static bool PositiveStatusCode(int code);

	public:
		static void Configure();
		static void Clean();
		
	public:
		RepHdr();

		virtual void reset();

		bool polyHeaders() const { return theXactId || theTarget; }
		bool expectPolyHeaders() const;
		bool expectBody() const;

		bool redirect() const;

		Time calcLmt() const;

		virtual void store(OLog &log) const;

		bool acceptedEncoding(int coding) const;

	protected:
		virtual bool parseRLine(const char *buf, const char *eol);

		virtual bool parseServer(const char *buf, const char *eoh);
		virtual bool parseProxyAuthenticate(const char *buf, const char *eoh);
		virtual bool parseWwwAuthenticate(const char *buf, const char *eoh);
		virtual bool parseLocation(const char *buf, const char *eoh);
		virtual bool parseLMT(const char *buf, const char *eoh);
		virtual bool parseExpires(const char *buf, const char *eoh);
		virtual bool parseContRange(const char *buf, const char *eoh);

	protected:
		static MsgHdrParsTab *TheParsTab;

	public:
		HttpUri theLocn;  // Location: in 3xx responses
		Time theLMT;      // Last-Modified-Time header field
		Time theExpires;  // Expires HTTP header field
		int theStatus;    // HTTP status code

		String theServer; // server agent string
		AuthChallenge theProxyAuthenticate; // proxy authenticate header
		AuthChallenge theOriginAuthenticate; // origin authenticate header
		Size theContRangeFirstByte;
		Size theContRangeLastByte;
		Size theContRangeInstanceLength;
};

inline ostream &crlf(ostream &os) { return os.write("\r\n", 2); }

#endif
