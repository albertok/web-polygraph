
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "runtime/globals.h"
#include "runtime/polyErrors.h"
#include "icp/IcpMsg.h"
#include "icp/IcpBase.h"


IcpBase::~IcpBase() {
	if (theReserv)
		TheFileScanner->clearRes(theReserv);
	if (theSock)
		theSock.close();
}

void IcpBase::configure(const NetAddr &aHost) {
	theHost = aHost;
}

void IcpBase::start() {
	Must(theSock.create(AF_INET, SOCK_DGRAM, 0));
	Must(theSock.blocking(false));
	Should(theSock.reuseAddr(true));

	if (!theSock.bind(theHost)) {
		Comment(0) << "ICP agent cannot listen on " << theHost << ": " << Error::Last() << endc;
		FatalError(errOther);
	}

	Assert(!theReserv);
	theReserv = TheFileScanner->setFD(theSock.fd(), dirRead, this);
}

void IcpBase::noteReadReady(int fd) {
	Assert(fd == theSock.fd());
	IcpMsg m;
	while (m.receive(theSock)) {
		if (m.opCode() == icpQuery)
			noteRequest(m);
		else
			noteReply(m);
	}
}

void IcpBase::noteRequest(const IcpMsg &m) {
	noteUnexpMsg(m);
}

void IcpBase::noteReply(const IcpMsg &m) {
	noteUnexpMsg(m);
}

void IcpBase::noteUnexpMsg(const IcpMsg &m) {
	if (ReportError(errIcpUnexpMsg))
		Comment(0) << "ICP agent on " << theHost << " got opcode " << m.opCode() << " from " << m.peer() << endc;
}
