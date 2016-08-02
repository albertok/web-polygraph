
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/httpText.h"

#include <list>


typedef std::list<String> Headers;
static Headers TheHeaders;

static
const String LearnHeader(const String &s) {
	Assert(s.str(": "));
	TheHeaders.push_back(s);
	return s;
}

bool KnownHeader(const String &header) {
	bool exists = false;
	typedef Headers::const_iterator HCI;
	for (HCI i = TheHeaders.begin(); !exists && i != TheHeaders.end(); ++i) {
		if (i->startsWith(header + ':'))
			exists = true;
	}
	return exists;
}


// protocol strings
const String protoHttp1p0 = "HTTP/1.0";
const String protoHttp1p1 = "HTTP/1.1";

// request line prefixes
const String rlpGet = "GET ";
const String rlpHead = "HEAD ";
const String rlpPost = "POST ";
const String rlpPut = "PUT ";
const String rlpConnect = "CONNECT ";

// request line suffixes
const String rlsHttp1p0 = " HTTP/1.0\r\n";
const String rlsHttp1p1 = " HTTP/1.1\r\n";

// response line suffix: status code and reason phrase
const String rls100Continue = " 100 Continue\r\n";
const String rls200Ok = " 200 OK\r\n";       
const String rls206PartialContent = " 206 Partial Content\r\n";       
const String rls302Found = " 302 Found\r\n";
const String rls304NotModified = " 304 Not Modified\r\n";
const String rls406NotAcceptable = " 406 Not Acceptable\r\n";
const String rls416RequestedRangeNotSatisfiable = " 416 Requested Range Not Satisfiable\r\n";
const String rls417ExpectationFailed = " 417 Expectation Failed\r\n";

// full header fields
const String hfAccept       = LearnHeader("Accept: */*\r\n");
const String hfConnAliveOrg = LearnHeader("Connection: keep-alive\r\n");
const String hfConnCloseOrg = LearnHeader("Connection: close\r\n");
const String hfConnAlivePxy = LearnHeader("Proxy-Connection: keep-alive\r\n");
const String hfConnClosePxy = LearnHeader("Proxy-Connection: close\r\n");
const String hfCcReload     = LearnHeader("Cache-Control: no-cache\r\n");
const String hfPragmaReload = LearnHeader("Pragma: no-cache\r\n");
const String hfCcCachable   = LearnHeader("Cache-Control: public\r\n");
const String hfCcUncachable = LearnHeader("Cache-Control: private,no-cache\r\n");
const String hfPragmaUncachable = LearnHeader("Pragma: no-cache\r\n");
const String hfExpect100Continue = LearnHeader("Expect: 100-continue\r\n");

// header field prefixes
const String hfpAcceptEncoding = LearnHeader("Accept-Encoding: ");
const String hfpCacheControl = LearnHeader("Cache-Control: ");
const String hfpConnection   = LearnHeader("Connection: ");
const String hfpCookie       = LearnHeader("Cookie: ");
const String hfpPragma       = LearnHeader("Pragma: ");
const String hfpProxyConnection = LearnHeader("Proxy-Connection: ");
const String hfpProxyAuthenticate = LearnHeader("Proxy-Authenticate: ");
const String hfpWwwAuthenticate = LearnHeader("WWW-Authenticate: ");
const String hfpTransferEncoding = LearnHeader("Transfer-Encoding: ");
const String hfpSetCookie    = LearnHeader("Set-Cookie: ");
const String hfpHost         = LearnHeader("Host: ");
const String hfpIMS          = LearnHeader("If-Modified-Since: ");
const String hfpContentEncoding   = LearnHeader("Content-Encoding: ");
const String hfpContLength   = LearnHeader("Content-Length: ");
const String hfpContType     = LearnHeader("Content-Type: ");
const String hfpContMd5      = LearnHeader("Content-MD5: ");
const String hfpDate         = LearnHeader("Date: ");
const String hfpExpires      = LearnHeader("Expires: ");
const String hfpLmt          = LearnHeader("Last-Modified: ");
const String hfpServer       = LearnHeader("Server: ");
const String hfpLocation     = LearnHeader("Location: ");
const String hfpProxyAuthorization = LearnHeader("Proxy-Authorization: ");
const String hfpAuthorization = LearnHeader("Authorization: ");
const String hfpXLocWorld    = LearnHeader("X-Loc-World: ");
const String hfpXRemWorld    = LearnHeader("X-Rem-World: ");
const String hfpXXact        = LearnHeader("X-Xact: ");
const String hfpXTarget      = LearnHeader("X-Target: ");
const String hfpXAbort       = LearnHeader("X-Abort: ");
const String hfpXPhaseSyncPos= LearnHeader("X-Phase-Sync-Pos: ");
const String hfpContRange    = LearnHeader("Content-Range: bytes ");
const String hfpRange        = LearnHeader("Range: bytes=");
const String hfpExpect       = LearnHeader("Expect: ");
const String hfpContDisposition = LearnHeader("Content-Disposition: attachment; filename=");

// free text
const String text302Found  = "please go to ";
const String text406NotAcceptable =
	"None of client-supported content codings are supported by the server.\n";
const String text416RequestedRangeNotSatisfiable =
	"Range specified in request is not satisfiable.\n";
const String text417ExpectationFailed =
	"Expectation failed.\n";
const String textMultipartBoundary = "THIS_STRING_SEPARATES";
const String textMultipartSep = "--";

// combos
const String hfGzipContentEncoding = LearnHeader(hfpContentEncoding + "gzip\r\n");
const String hfVaryAcceptEncoding = LearnHeader("Vary: Accept-Encoding\r\n");
const String hfMultiRangeContType = LearnHeader(hfpContType + "multipart/byteranges; boundary=" + textMultipartBoundary + "\r\n");
