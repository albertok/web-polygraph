
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/FtpText.h"

// requests
const String ftpReqUserPfx = "USER ";
const String ftpReqUserAnonym = "anonymous";
const String ftpReqUserSfx = "\r\n";
const String ftpReqPassPfx = "PASS ";
const String ftpReqPassAnonym = "anonymous@";
const String ftpReqPassSfx = "\r\n";
const String ftpReqTypeBinary = "TYPE I\r\n";
const String ftpReqPasv = "PASV\r\n";
const String ftpReqPort = "PORT ";
const String ftpReqRetr = "RETR ";
const String ftpReqStor = "STOR ";
const String ftpReqQuit = "QUIT\r\n";

// responses
const String ftpRspGreeting = "220 Features: .\r\n";
const String ftpRspSystem = "215 UNIX type: L8\r\n";
const String ftpRspUser = "331 username accepted, password required.\r\n";
const String ftpRspPass = "230 password accepted.\r\n";
const String ftpRspTypeBinary = "200 entering BINARY mode.\r\n";
const String ftpRspMdtm = "213 ";
const String ftpRspSize = "213 ";
const String ftpRspCwd = "250 Okay.\r\n";
const String ftpRspPwdPfx = "257 \"";
const String ftpRspPwdSfx = "\" is current directory.\r\n";
const String ftpRspPasvPfx = "227 Entering Passive Mode (";
const String ftpRspPasvSfx = ")\r\n";
const String ftpRspPort = "200 PORT command accepted.\r\n";
const String ftpRspXferOpening = "150 File status okay; waiting for data connection.\r\n";
const String ftpRspXferStarting = "125 Data connection already open; transfer starting.\r\n";
const String ftpRspXferComplete = "226 Closing data connection.\r\n";
const String ftpRspXferNoDataConn = "425 No data connection established.\r\n";
const String ftpRspXferAborted = "426 failure while transferring data.\r\n";
const String ftpRspBadUrl = "551 bad URL.\r\n";
const String ftpRspQuit = "221 Bye.\r\n";
const String ftpRspCmdNotImplemented = "502 Command not implemented.\r\n";
