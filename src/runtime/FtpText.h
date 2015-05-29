
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_HTTPTEXT_H
#define POLYGRAPH__RUNTIME_HTTPTEXT_H

#include "xstd/String.h"

// various FTP strings

// requests

extern const String ftpReqUserPfx;
extern const String ftpReqUserAnonym;
extern const String ftpReqUserSfx;
extern const String ftpReqPassPfx;
extern const String ftpReqPassAnonym;
extern const String ftpReqPassSfx;
extern const String ftpReqTypeBinary;
extern const String ftpReqPasv;
extern const String ftpReqPort;
extern const String ftpReqRetr;
extern const String ftpReqStor;
extern const String ftpReqQuit;

// responses; TODO: s/ftpRsp/ftpRep/ to follow Polygraph naming scheme
extern const String ftpRspGreeting;
extern const String ftpRspSystem;
extern const String ftpRspUser;
extern const String ftpRspPass;
extern const String ftpRspTypeBinary;
extern const String ftpRspMdtm;
extern const String ftpRspSize;
extern const String ftpRspCwd;
extern const String ftpRspPwdPfx;
extern const String ftpRspPwdSfx;
extern const String ftpRspPasvPfx;
extern const String ftpRspPasvSfx;
extern const String ftpRspPort;
extern const String ftpRspXferOpening;
extern const String ftpRspXferStarting;
extern const String ftpRspXferComplete;
extern const String ftpRspXferNoDataConn;
extern const String ftpRspXferAborted;
extern const String ftpRspBadUrl;
extern const String ftpRspQuit;
extern const String ftpRspCmdNotImplemented;

#endif
