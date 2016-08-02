
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/polyLogCats.h"
#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "icp/IcpMsg.h"
#include "icp/IcpClient.h"

#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"


/* IcpCltUser */

IcpCltUser::~IcpCltUser() {
	Assert(!theReserv);
}

void IcpCltUser::reset() {
	theReserv.reset();
}


/* IcpClient */

IcpClient::IcpClient(): theLastReqNum(0) {
	// use some large number to have enough slots
	// for most request rates; 
	// make this configurable if needed
	theUsers.resize(8*1024);
}

IcpCltRes IcpClient::expectReply(IcpCltUser *u) {
	if (++theLastReqNum >= theUsers.capacity())
		theLastReqNum = 1;
	theUsers[theLastReqNum] = u;
	return IcpCltRes(theLastReqNum);
}

void IcpClient::cancel(IcpCltRes &res) {
	Assert(res);
	Assert(res.reqNum() < theUsers.capacity());
	theUsers[res.reqNum()] = 0;
	res.reset();
}

void IcpClient::noteReply(const IcpMsg &m) {
	const int rn = m.reqNum();
	if (rn < 0 || rn >= theUsers.capacity()) {
		ReportError(errIcpBadReqNum);
		return;
	}

	if (rn == 0) {
		// warn once about the misbehaving peer
		static int warnCount = 0;
		if (!warnCount++)
			Comment(1) << "warning: ICP peer " << m.peer() << " does not bounce reqnums" << endc;
		// XXX: implement efficient url->user mapping
		return;
	}

	if (IcpCltUser *u = theUsers[rn]) {
		if (u->oid() != m.oid()) {
			ReportError(errIcpRepOverlap);
			return;
		}

		u->noteReply(m);
		return;
	}

	// else probably timedout
}

int IcpClient::logCat() const {
	return lgcCltSide;
}
