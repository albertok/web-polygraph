
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/httpText.h"

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
const String hfAccept       = "Accept: */*\r\n";
const String hfConnAliveOrg = "Connection: keep-alive\r\n";
const String hfConnCloseOrg = "Connection: close\r\n";
const String hfConnAlivePxy = "Proxy-Connection: keep-alive\r\n";
const String hfConnClosePxy = "Proxy-Connection: close\r\n";
const String hfReload       = "Pragma: no-cache\r\n"
                                     "Cache-Control: no-cache\r\n";
const String hfCcCachable   = "Cache-Control: public\r\n";
const String hfCcUncachable = "Cache-Control: private,no-cache\r\n"
                                     "Pragma: no-cache\r\n";
const String hfExpect100Continue = "Expect: 100-continue\r\n";

// header field prefixes
const String hfpAcceptEncoding = "Accept-Encoding: ";
const String hfpCacheControl = "Cache-Control: ";
const String hfpConnection   = "Connection: ";
const String hfpCookie       = "Cookie: ";
const String hfpPragma       = "Pragma: ";
const String hfpProxyConnection = "Proxy-Connection: ";
const String hfpProxyAuthenticate = "Proxy-Authenticate: ";
const String hfpWwwAuthenticate = "WWW-Authenticate: ";
const String hfpTransferEncoding = "Transfer-Encoding: ";
const String hfpSetCookie    = "Set-Cookie: ";
const String hfpHost         = "Host: ";
const String hfpIMS          = "If-Modified-Since: ";
const String hfpContentEncoding   = "Content-Encoding: ";
const String hfpContLength   = "Content-Length: ";
const String hfpContType     = "Content-Type: ";
const String hfpContMd5      = "Content-MD5: ";
const String hfpDate         = "Date: ";
const String hfpExpires      = "Expires: ";
const String hfpLmt          = "Last-Modified: ";
const String hfpServer       = "Server: ";
const String hfpLocation     = "Location: ";
const String hfpProxyAuthorization = "Proxy-Authorization: ";
const String hfpAuthorization = "Authorization: ";
const String hfpXLocWorld    = "X-Loc-World: ";
const String hfpXRemWorld    = "X-Rem-World: ";
const String hfpXXact        = "X-Xact: ";
const String hfpXTarget      = "X-Target: ";
const String hfpXAbort       = "X-Abort: ";
const String hfpXPhaseSyncPos= "X-Phase-Sync-Pos: ";
const String hfpContRange    = "Content-Range: bytes ";
const String hfpRange        = "Range: bytes=";
const String hfpExpect       = "Expect: ";
const String hfpContDisposition = "Content-Disposition: attachment; filename=";

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
const String hfGzipContentEncoding = hfpContentEncoding + "gzip\r\n";
const String hfVaryAcceptEncoding = "Vary: Accept-Encoding\r\n";
const String hfMultiRangeContType = hfpContType + "multipart/byteranges; boundary=" + textMultipartBoundary + "\r\n";
