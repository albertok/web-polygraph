
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_HTTPTEXT_H
#define POLYGRAPH__RUNTIME_HTTPTEXT_H

#include "xstd/String.h"

extern bool KnownHeader(const String &header);

// various HTTP header strings

// protocol strings
extern const String protoHttp1p0;
extern const String protoHttp1p1;

// request line prefixes
extern const String rlpGet;
extern const String rlpHead;
extern const String rlpPost;
extern const String rlpPut;
extern const String rlpConnect;

// request line suffixes: protocol version and spaces
extern const String rlsHttp1p0;
extern const String rlsHttp1p1;

// response line suffix: status code and reason phrase
extern const String rls100Continue;
extern const String rls200Ok;
extern const String rls206PartialContent;
extern const String rls302Found;
extern const String rls304NotModified;
extern const String rls406NotAcceptable;
extern const String rls416RequestedRangeNotSatisfiable;
extern const String rls417ExpectationFailed;

// full header fields
extern const String hfAccept;
extern const String hfConnAliveOrg;
extern const String hfConnCloseOrg;
extern const String hfConnAlivePxy;
extern const String hfConnClosePxy;
extern const String hfCcReload;
extern const String hfPragmaReload;
extern const String hfCcCachable;
extern const String hfCcUncachable;
extern const String hfPragmaUncachable;
extern const String hfExpect100Continue;

// header field prefixes
extern const String hfpAcceptEncoding;
extern const String hfpCacheControl;
extern const String hfpConnection;
extern const String hfpCookie;
extern const String hfpPragma;
extern const String hfpProxyConnection;
extern const String hfpProxyAuthenticate;
extern const String hfpWwwAuthenticate;
extern const String hfpSetCookie;
extern const String hfpTransferEncoding;
extern const String hfpHost;
extern const String hfpIMS;
extern const String hfpXLocWorld;
extern const String hfpXRemWorld;
extern const String hfpXXact;
extern const String hfpXTarget;
extern const String hfpXAbort;
extern const String hfpXPhaseSyncPos;
extern const String hfpContentEncoding;
extern const String hfpContLength;
extern const String hfpContMd5;
extern const String hfpContType;
extern const String hfpDate;
extern const String hfpExpires;
extern const String hfpLmt;
extern const String hfpServer;
extern const String hfpLocation;
extern const String hfpProxyAuthorization;
extern const String hfpAuthorization;
extern const String hfpContRange;
extern const String hfpRange;
extern const String hfpExpect;
extern const String hfpContDisposition;

// free text
extern const String text302Found;
extern const String text406NotAcceptable;
extern const String text416RequestedRangeNotSatisfiable;
extern const String text417ExpectationFailed;
extern const String textMultipartBoundary;
extern const String textMultipartSep;

// known content-encoding tokens
typedef enum { codingIdentity, codingGzip, codingEnd } ContentCoding;
extern const String hfGzipContentEncoding;
extern const String hfVaryAcceptEncoding;
extern const String hfMultiRangeContType;

#endif
