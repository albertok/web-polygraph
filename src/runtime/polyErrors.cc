
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/polyErrors.h"

const Error errForeignSrc = Error::Add("foreign HTTP request or response");
const Error errForeignUrl = Error::Add("foreign URL");
const Error errHostlessForeignUrl = Error::Add("foreign URL without host name");
const Error errIcpForeignReq = Error::Add("foreign ICP request");
const Error errIcpForeignRep = Error::Add("foreign ICP reply");
const Error errMisdirRequest = Error::Add("misdirected request");
const Error errNoHostName = Error::Add("missing host name in the request");
const Error errForeignHostName = Error::Add("foreign host name");
const Error errBadHostName = Error::Add("failed to parse host name");
const Error errSrvRedirect = Error::Add("unexpected server redirection");
const Error errRedirLocation = Error::Add("bad redirect location");
const Error errForeignTarget = Error::Add("request for unknown origin server");
const Error errNoTarget = Error::Add("missing target info in the request");
const Error errPrematureEof = Error::Add("premature end of msg body");
const Error errPrematureEoh = Error::Add("premature end of msg header");
const Error errExtraRepData = Error::Add("extra reply data");
const Error errNoHdrClose = Error::Add("connection closed before sending headers");
const Error errHttpRLine = Error::Add("malformed HTTP request or response line");
const Error errHttpStatusCode = Error::Add("unsupported HTTP status code");
const Error errReqBodyButNoCLen = Error::Add("missing Content-Length header in request with a body");
const Error errPersistButNoCLen = Error::Add("missing Content-Length header on a persistent connection");
const Error errUnexpectedCLen = Error::Add("unexpected Content-Length header");
const Error errUnexpectedCRange = Error::Add("unexpected Content-Range header");
const Error errPartContBadByteRange = Error::Add("malformed byte range");
const Error errPartContBadInstanceLen = Error::Add("malformed instance length");
const Error errPartContBadCountOrSize = Error::Add("response range count or size is larger than in request");
const Error errHttpNoDate = Error::Add("missing Date: header field");
const Error errSyncDate = Error::Add("clocks out of sync");
const Error errHugeHdr = Error::Add("HTTP header is too big");
const Error errUnchbHit = Error::Add("hit on uncachable object");
const Error err1UseHit = Error::Add("hit on first request to an object");
const Error errReloadHit = Error::Add("hit on reload request");
const Error errStaleHit = Error::Add("stale object");
const Error errChbChange = Error::Add("cachability status changed");
const Error errFalseHit = Error::Add("false hit");
const Error errFalseMiss = Error::Add("false miss");
const Error errServerGone = Error::Add("server had to terminate");
const Error errLogBufFull = Error::Add("log buffer is full");
const Error errUnclaimOid = Error::Add("an advertised oid has not been requested for a while");
const Error errUnsolicOid = Error::Add("a non-advertised oid has been requested");
const Error errNoNewOids = Error::Add("client ran out of new public oids");
const Error errFidFlood = Error::Add("cannot send failed oids to servers fast enough");
const Error errNidReqFlood = Error::Add("server received too many requests for new oids");
const Error errNidRepFlood = Error::Add("client received too many new oids");
const Error errNidStoreLimit = Error::Add("new oid storage cannot grow any more");
const Error errSrvChangedWid = Error::Add("client discovered server world id change");
const Error errUnreachContType = Error::Add("unreachable content type");
const Error errForeignTag = Error::Add("foregn content <tag>");
const Error errMalformedTag = Error::Add("malformed content <tag>");
const Error errOpenTag = Error::Add("open content <tag"); // vs. unbalanced
const Error errTagAttrValueExpected = Error::Add("attribute value expected in a content <tag>");
const Error errTagAttrValueOpen = Error::Add("unterminated attribute value expected in a content <tag>");
const Error errEmptyTag = Error::Add("empty content tag, <>");
const Error errTagnameValue = Error::Add("found <attr=value> tag; missing tagname?");
const Error errBadEmbedUri = Error::Add("bad URI in content <tag> attribute");
const Error errTooManyWaitXact = Error::Add("too many postponed xactions");
const Error errXactLifeTimeExpired = Error::Add("xaction lifetime expired");
const Error errTimingDrift = Error::Add("internal timers may be getting behind");
const Error errSiblingViolation = Error::Add("violation of a sibling relationship");
const Error errMake302Found = Error::Add("``302 Found'' reply generation failed");
const Error errUnderpopulated = Error::Add("cannot meet robot population goal");
const Error errChecksum = Error::Add("message content checksum mismatch");
const Error errConnectEstb = Error::Add("failed to establish a connection");
const Error errUnknownTransferEncoding = Error::Add("transfer codings other than 'chunked' and 'identity' are not supported");
const Error errContentLeftovers = Error::Add("content syntax error at end of message body");
const Error errHugeContentToken = Error::Add("huge content token in message body");
const Error errUnknownEmbedCategory = Error::Add("unknown category of embedded content");
const Error errAclNoMatches = Error::Add("no access control rules matched");
const Error errAclWrongMatch = Error::Add("wrong access control rule matched at the intermediary?");
const Error errAclManyMatches = Error::Add("several access control rules matched");
const Error errAclHostIpLookup = Error::Add("IP unavailable for access rule with host_ip scope");
const Error errChunkHugeToken = Error::Add("huge token in chunked encoded message");
const Error errChunkNegativeSize = Error::Add("chunk with a negative size");
const Error errChunkSize = Error::Add("cannot parse chunk size");
const Error errChunkNextParser = Error::Add("internal error while parsing unchunked content");
const Error errChunkSuffix = Error::Add("garbage between chunk data and CRLF chunk terminator");
const Error errTrailerHeader = Error::Add("ignoring HTTP trailer header");
const Error errChunkedButCLen = Error::Add("ignoring disallowed Content-Length header with chunked transfer encoding");
const Error errManyRetries = Error::Add("transaction failed after many retries");
const Error errPipelineAbort = Error::Add("aborting pipelined transaction due to pipeline errors");
const Error errReqDontFit = Error::Add("generated request headers too big for write buffer");
const Error errCookiesDontFit = Error::Add("generated cookies too big for write buffer");
const Error errNoAcceptableContentCoding = Error::Add("no content coding acceptable to requester is supported");
const Error errMultipartBoundary = Error::Add("multipart with bad boundary");
const Error errMultipartNextParser = Error::Add("internal error while parsing maltipart body content");
const Error errMultipartSuffix = Error::Add("garbage between multipart body data and CRLF terminator");
const Error errMultipartHugeToken = Error::Add("huge token in multipart message");
const Error errUnexpected100Continue = Error::Add("unexpected \"100 Continue\" response");
const Error errUnsupportedControlMsg = Error::Add("unsupported control message (1xx status code)");

const Error errForbiddenWoutCreds = Error::Add("access forbidden to an anonymous robot");
const Error errForbiddenBeforeAuth = Error::Add("access forbidden before authentication was started");
const Error errForbiddenDuringAuth = Error::Add("access forbidden while authentication was in progress");
const Error errForbiddenAfterAuth = Error::Add("access forbidden after authentication was completed");
const Error errProxyAuthHeaders = Error::Add("proxy authentication without authenticate headers");
const Error errOriginAuthHeaders = Error::Add("origin authentication without authenticate headers");
const Error errProxyAuthWoutCreds = Error::Add("proxy authentication with anonymous robot");
const Error errOriginAuthWoutCreds = Error::Add("origin authentication with anonymous robot");
const Error errProxyAuthAfterAuth = Error::Add("proxy re-authentication requested after authentication");
const Error errOriginAuthAfterAuth = Error::Add("origin re-authentication requested after authentication");
const Error errProxyAuthAllowed = Error::Add("proxy authentication failed: access allowed with invalid credentials");
const Error errOriginAuthAllowed = Error::Add("origin authentication failed: access allowed with invalid credentials");
const Error errAuthBug = Error::Add("internal authentication state error");

const Error errSocksRead = Error::Add("SOCKS read failure");
const Error errSocksWrite = Error::Add("SOCKS write failure");
const Error errSocksUnexpectedEof = Error::Add("SOCKS protocol-violating EOF on read");
const Error errSocksReqDontFit = Error::Add("SOCKS request too big for write buffer");
const Error errSocksRepDontFit = Error::Add("SOCKS reply too big for read buffer");
const Error errSocksVersion = Error::Add("unsupported SOCKS version");
const Error errSocksAuthUserPassVersion = Error::Add("unsupported SOCKS username/password authentication version");
const Error errSocksAuthUserPassWoutCreds = Error::Add("SOCKS proxy requested Username/Password authentication method but no credentials configured");
const Error errSocksAuthUnsupported = Error::Add("SOCKS proxy requested unsupported authentication method");
const Error errSocksAuthInvalid = Error::Add("no acceptable methods were offered to SOCKS proxy");
const Error errSocksAuthForbidden = Error::Add("SOCKS proxy access forbidden");
const Error errSocksConnect = Error::Add("SOCKS connect failure");

const Error errSslCloseTout = Error::Add("non-blocking SSL close operation time out");
const Error errSslSessionResume = Error::Add("failed to resume an SSL session, continuing anyway");
const Error errSslIo = Error::Add("SSL I/O failure");

const Error errIcpRepCode = Error::Add("unsupported ICP opcode");
const Error errIcpVersion = Error::Add("unsupported ICP version");
const Error errIcpMsgSize = Error::Add("bad ICP message size");
const Error errIcpBadReqNum = Error::Add("bad ICP reqnum");
const Error errIcpRepOverlap = Error::Add("ICP client may have too many outstanding requests");
const Error errIcpUnexpMsg = Error::Add("unexpected message to an ICP agent");
const Error errIcpErrOpcode = Error::Add("got ICP reply with ICP_ERROR opcode");

const Error errDnsBadMsgId = Error::Add("DNServer responded with invalid message ID");
const Error errDnsAllSrvsFailed = Error::Add("all DNServers failed to resolve a domain name");
const Error errDnsRepTimeout = Error::Add("timedout waiting for DNServer response");
const Error errDnsRepMismatch = Error::Add("DNServer response does not match our query");
const Error errDnsRespFmt = Error::Add("failed to parse DNServer response");
const Error errDnsQueryFmt = Error::Add("DNServer failed to parse our query");
const Error errDnsSrvFail = Error::Add("DNServer failed to process our query");
const Error errDnsNoName = Error::Add("DNServer did not resolve domain name");
const Error errDnsNoSupp = Error::Add("DNServer does not support the requested operation");
const Error errDnsRefused = Error::Add("DNServer refused to process the query");
const Error errDnsRCode = Error::Add("unknown response code from DNServer");

const Error errFtpHugeCmd = Error::Add("FTP command is too big");
const Error errFtpPrematureEndOfMsg = Error::Add("premature end of FTP message");
const Error errFtpPrematureEndOfCtrl = Error::Add("premature closure of FTP control channel");
const Error errFtpNoArgument = Error::Add("missing FTP command argument");
const Error errFtpMode = Error::Add("unsupported transfer mode");
const Error errFtpCommandNotImplemented = Error::Add("FTP command not implemented");
const Error errFtpCmdSequence = Error::Add("bad FTP command sequence or state");
const Error errFtpCommandFailed = Error::Add("FTP command failed");
const Error errFtpBadPasv = Error::Add("bad FTP PASV reply");
const Error errFtpBadPort = Error::Add("bad FTP PORT request");
const Error errFtpNoDataXfer = Error::Add("FTP exchange terminated w/o data transfer");

const Error errGssContextCreate = Error::Add("failed to create GSS context");

const Error errKerberosAllSrvsFailed = Error::Add("all KDC servers failed");
const Error errKerberosAuthFailed = Error::Add("Kerberos HTTP authentication failed");
const Error errKerberosCredsWithMacros = Error::Add("cannot use credentials with macros for Kerberos; robot transactions that need Kerberos will fail");
const Error errKerberosCredsInit = Error::Add("cannot initialize robot Kerberos credentials or ccache");
const Error errKerberosKdcTimeout = Error::Add("timedout waiting for KDC response");
const Error errKerberosPrincipal = Error::Add("invalid Kerberos principal");
const Error errKerberosTicket = Error::Add("cannot acquire a Kerberos ticket");

const Error errKerberosAsInit = Error::Add("cannot initiate a Kerberos AS request; robot transactions that need Kerberos will fail");
const Error errKerberosTgt = Error::Add("cannot acquire Kerberos TGT");
const Error errKerberosNoTgt = Error::Add("missing Kerberos TGT");
const Error errKerberosTcpPfx = Error::Add("malformed TCP prefix in KDC reply");


const Error errNegativePhase = Error::Add("too many errors");
const Error errPglScript = Error::Add("PGL runtime script error");
const Error errOther = Error::Add("unclassified error");

const Error errKerberosCtxState = Error::Add("bad Kerberos context state");
const Error errNegotiateOrigin = Error::Add("unsupported Negotiate authentication with an origin server");
const Error errKerberosKdcEof = Error::Add("Unexpected KDC socket closure");
const Error errAuthHeaderClash = Error::Add("generated authentication headers clash with user-configured headers");
