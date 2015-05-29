
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/Client.h"
#include "client/CltXact.h"
#include "runtime/ErrorMgr.h"
#include "icp/IcpMsg.h"
#include "icp/IcpClient.h"
#include "client/IcpCltXact.h"

#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "base/polyLogCats.h"


Time IcpCltXact::TheTimeout;

void IcpCltXact::reset() {
	Assert(!thePendAlarmCnt);
	IcpXaction::reset();
	IcpCltUser::reset();

	theOwner = 0;
	theAddr = NetAddr();
	theOid.reset();
	theStartTime = theLifeTime = Time();
	theRepSize = Size();
	theResult = icpInvalid;
	theReason = 0;
	theLogCat = lgcCltSide;
}

void IcpCltXact::exec(Client *anOwner, const NetAddr &anAddr) {
	theOwner = anOwner;
	theAddr = anAddr;
	theOid = theReason->oid();
	Assert(icpClient());

	IcpXaction::start();

	theReserv = icpClient()->expectReply(this);

	IcpMsg r;
	r.opCode(icpQuery);
	r.oid(theOid);
	r.reqNum(theReserv.reqNum());
	r.peer(theAddr);

	if (!r.send(icpClient()->socket()))
		finish(Error::Last());

	if (TheTimeout >= 0)
		sleepFor(TheTimeout);
}

void IcpCltXact::wakeUp(const Alarm &a) {
	AlarmUser::wakeUp(a);
	cerr << here << "ICP query to " << theAddr << " timedout after " << TheTimeout << endl;
	theResult = icpTimeout;
	finish(0);
}

void IcpCltXact::noteReply(const IcpMsg &r) {
	theResult = r.opCode();
	theRepSize = r.size();
	switch (r.opCode()) {
		case icpHit:
		case icpMiss:
		case icpMissNoFetch:
				finish(0);
				break;
		default:
				finish(errIcpRepCode);
	}
}

IcpClient *IcpCltXact::icpClient() {
	return theOwner->icpClient();
}

void IcpCltXact::finish(Error err) {
	IcpXaction::finish(err);

	if (theReserv)
		icpClient()->cancel(theReserv);
	cancelAlarms();

	theOwner->notePeerAsked(this);
}
