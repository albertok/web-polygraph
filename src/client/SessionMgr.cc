
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "client/Client.h"
#include "client/CltCfg.h"
#include "client/CltXact.h"
#include "client/SessionMgr.h"

class SessionHeartbitMgr: public AlarmUser {
	public:
		SessionHeartbitMgr(SessionMgr *anOwner, Time aGap);
		virtual ~SessionHeartbitMgr();

		virtual void wakeUp(const Alarm &a);

	protected:
		SessionMgr *theOwner;
		Time theGap;
};


/* SessionHeartbitMgr */

SessionHeartbitMgr::SessionHeartbitMgr(SessionMgr *anOwner, Time aGap):
	theOwner(anOwner), theGap(aGap) {
	Assert(theOwner && theGap >= 0);
	sleepFor(theGap);
}

SessionHeartbitMgr::~SessionHeartbitMgr() {
	cancelAlarms();
}

void SessionHeartbitMgr::wakeUp(const Alarm &a) {
	AlarmUser::wakeUp(a);
	Assert(theOwner);
	theOwner->bitHeart();
	sleepFor(theGap);
}


/* SessionMgr */

SessionMgr::SessionMgr(Client *anOwner): theOwner(anOwner), theCfg(0),
	theHeartbitMgr(0), isIdle(false) {
	Assert(theOwner);
}

SessionMgr::~SessionMgr() {
	stopHeartbit();
}

void SessionMgr::configure(const CltCfg *aCfg) {
	Assert(!theCfg && aCfg);
	theCfg = aCfg;
}

void SessionMgr::delayedStart(Time delay) {
	// fake idle state
	isIdle = true;
	sleepFor(delay);
}

void SessionMgr::start() {
	becomeBusy();
}

void SessionMgr::stop() {
	becomeIdle();
	cancelAlarms();
}

void SessionMgr::stopHeartbit() {
	if (theHeartbitMgr) {
		delete theHeartbitMgr;
		theHeartbitMgr = 0;
	}
}

void SessionMgr::becomeBusy() {
	Assert(theOwner);
	theStats.restart();
	isIdle = false;
	theOwner->becomeBusy();

	if (theCfg->theBusyPeriod->duration() >= 0)
		sleepFor(theCfg->theBusyPeriod->duration());

	Assert(!theHeartbitMgr);
	if (theCfg->theSessionHeartbitGap >= 0)
		theHeartbitMgr = new SessionHeartbitMgr(this, theCfg->theSessionHeartbitGap);
}

void SessionMgr::bitHeart() {
	Assert(theOwner);
	theOwner->continueSession();
}

void SessionMgr::becomeIdle() {
	Assert(theOwner);
	theOwner->becomeIdle();
	stopHeartbit();
	isIdle = true;
	sleepFor(Time::Secd(Max(0.0, theCfg->theIdlePeriodDur->trial())));
}

void SessionMgr::noteXactDone(CltXact *x) {
	Assert(x);

	// update stats; XXX: this code does not account for compound transactions
	const ObjId &oid = x->oid();
	theStats.theXactCnt++;
	if (oid.fill())
		theStats.theFillSz += BigSize(x->repSize().actual());

	checkPoint(); // XXX: may re-enter the caller
}

// XXX: we do not support error-based session goal -- nobody calls this method
void SessionMgr::noteXactErr(CltXact *x) {
	Assert(x);

	// update stats
	theStats.theXactErrCnt++;

	checkPoint();
}

void SessionMgr::checkPoint() {
	if (!isIdle && theCfg->theBusyPeriod->reached(theStats))
		becomeIdle();
}

void SessionMgr::wakeUp(const Alarm &a) {
	AlarmUser::wakeUp(a);
	if (isIdle)
		becomeBusy();
	else
		becomeIdle();
}
