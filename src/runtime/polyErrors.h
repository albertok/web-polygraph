
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_POLYERRORS_H
#define POLYGRAPH__RUNTIME_POLYERRORS_H

extern const Error errNone;
extern const Error errForeignSrc;
extern const Error errForeignUrl;
extern const Error errHostlessForeignUrl;
extern const Error errIcpForeignReq;
extern const Error errIcpForeignRep;
extern const Error errMisdirRequest;
extern const Error errForeignHostName;
extern const Error errBadHostName;
extern const Error errNoHostName;
extern const Error errSrvRedirect;
extern const Error errRedirLocation;
extern const Error errForeignTarget;
extern const Error errNoTarget;
extern const Error errPrematureEof;
extern const Error errPrematureEoh;
extern const Error errExtraRepData;
extern const Error errHttpRLine;
extern const Error errHttpStatusCode;
extern const Error errNoHdrClose;
extern const Error errReqBodyButNoCLen;
extern const Error errPersistButNoCLen;
extern const Error errUnexpectedCLen;
extern const Error errUnexpectedCRange;
extern const Error errPartContBadByteRange;
extern const Error errPartContBadInstanceLen;
extern const Error errPartContBadCountOrSize;
extern const Error errHttpNoDate;
extern const Error errSyncDate;
extern const Error errHugeHdr;
extern const Error errUnchbHit;
extern const Error err1UseHit;
extern const Error errReloadHit;
extern const Error errStaleHit;
extern const Error errChbChange;
extern const Error errFalseHit;
extern const Error errFalseMiss;
extern const Error errServerGone;
extern const Error errLogBufFull;
extern const Error errUnclaimOid;
extern const Error errUnsolicOid;
extern const Error errNoNewOids;
extern const Error errFidFlood;
extern const Error errNidReqFlood;
extern const Error errNidRepFlood;
extern const Error errNidStoreLimit;
extern const Error errSrvChangedWid;
extern const Error errUnreachContType;
extern const Error errForeignTag;
extern const Error errMalformedTag;
extern const Error errOpenTag; // vs. unbalanced
extern const Error errTagAttrValueExpected;
extern const Error errTagAttrValueOpen;
extern const Error errEmptyTag;
extern const Error errTagnameValue;
extern const Error errBadEmbedUri;
extern const Error errTooManyWaitXact;
extern const Error errXactLifeTimeExpired;
extern const Error errTimingDrift;
extern const Error errSiblingViolation;
extern const Error errMake302Found;
extern const Error errUnderpopulated;
extern const Error errChecksum;
extern const Error errConnectEstb;
extern const Error errUnknownTransferEncoding;
extern const Error errContentLeftovers;
extern const Error errHugeContentToken;
extern const Error errUnknownEmbedCategory;
extern const Error errAclNoMatches;
extern const Error errAclWrongMatch;
extern const Error errAclManyMatches;
extern const Error errAclHostIpLookup;
extern const Error errChunkHugeToken;
extern const Error errChunkNegativeSize;
extern const Error errChunkSize;
extern const Error errChunkNextParser;
extern const Error errChunkSuffix;
extern const Error errTrailerHeader;
extern const Error errChunkedButCLen;
extern const Error errManyRetries;
extern const Error errPipelineAbort;
extern const Error errReqDontFit;
extern const Error errCookiesDontFit;
extern const Error errNoAcceptableContentCoding;
extern const Error errMultipartBoundary;
extern const Error errMultipartNextParser;
extern const Error errMultipartSuffix;
extern const Error errMultipartHugeToken;
extern const Error errUnexpected100Continue;
extern const Error errUnsupportedControlMsg;

extern const Error errForbiddenWoutCreds;
extern const Error errForbiddenBeforeAuth;
extern const Error errForbiddenDuringAuth;
extern const Error errForbiddenAfterAuth;
extern const Error errProxyAuthHeaders;
extern const Error errOriginAuthHeaders;
extern const Error errProxyAuthWoutCreds;
extern const Error errOriginAuthWoutCreds;
extern const Error errProxyAuthAfterAuth;
extern const Error errOriginAuthAfterAuth;
extern const Error errProxyAuthAllowed;
extern const Error errOriginAuthAllowed;
extern const Error errAuthHeaderClash;
extern const Error errAuthBug;

extern const Error errSocksRead;
extern const Error errSocksWrite;
extern const Error errSocksUnexpectedEof;
extern const Error errSocksReqDontFit;
extern const Error errSocksRepDontFit;
extern const Error errSocksVersion;
extern const Error errSocksAuthUserPassVersion;
extern const Error errSocksAuthUserPassWoutCreds;
extern const Error errSocksAuthUnsupported;
extern const Error errSocksAuthInvalid;
extern const Error errSocksAuthForbidden;
extern const Error errSocksConnect;

extern const Error errSslCloseTout;
extern const Error errSslSessionResume;
extern const Error errSslIo;

extern const Error errIcpRepCode;
extern const Error errIcpVersion;
extern const Error errIcpMsgSize;
extern const Error errIcpBadReqNum;
extern const Error errIcpRepOverlap;
extern const Error errIcpUnexpMsg;
extern const Error errIcpErrOpcode;

extern const Error errDnsBadMsgId;
extern const Error errDnsAllSrvsFailed;
extern const Error errDnsRepTimeout;
extern const Error errDnsRepMismatch;
extern const Error errDnsRespFmt;
extern const Error errDnsQueryFmt;
extern const Error errDnsSrvFail;
extern const Error errDnsNoName;
extern const Error errDnsNoSupp;
extern const Error errDnsRefused;
extern const Error errDnsRCode;

extern const Error errFtpHugeCmd;
extern const Error errFtpPrematureEndOfMsg;
extern const Error errFtpPrematureEndOfCtrl;
extern const Error errFtpNoArgument;
extern const Error errFtpMode;
extern const Error errFtpCommandNotImplemented;
extern const Error errFtpCmdSequence;
extern const Error errFtpCommandFailed;
extern const Error errFtpBadPasv;
extern const Error errFtpBadPort;
extern const Error errFtpNoDataXfer;

extern const Error errGssContextCreate;

extern const Error errNegotiateOrigin;

extern const Error errKerberosAllSrvsFailed;
extern const Error errKerberosAuthFailed;
extern const Error errKerberosCredsWithMacros;
extern const Error errKerberosCredsInit;
extern const Error errKerberosAsInit;
extern const Error errKerberosKdcTimeout;
extern const Error errKerberosPrincipal;
extern const Error errKerberosTicket;
extern const Error errKerberosCtxState;
extern const Error errKerberosTgt;
extern const Error errKerberosNoTgt;
extern const Error errKerberosTcpPfx;
extern const Error errKerberosKdcEof;

extern const Error errNegativePhase;
extern const Error errPglScript;
extern const Error errOther;

#endif
