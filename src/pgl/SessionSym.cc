
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglRateSym.h"
#include "pgl/GoalSym.h"
#include "pgl/SessionSym.h"



const String SessionSym::TheType = "Session";

static String strBusy_period = "busy_period";
static String strHeartbeat_notif_rate = "heartbeat_notif_rate";
static String strIdle_period_duration = "idle_period_duration";
static String strTime_distr = "time_distr";


SessionSym::SessionSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(GoalSym::TheType, strBusy_period, new GoalSym);
	theRec->bAdd(strTime_distr, strIdle_period_duration, 0);
	theRec->bAdd(RateSym::TheType, strHeartbeat_notif_rate, 0);
}

SessionSym::SessionSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool SessionSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *SessionSym::dupe(const String &type) const {
	if (isA(type))
		return new SessionSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

GoalSym *SessionSym::busyPeriod() const {
	SynSymTblItem *gi = 0;
	Assert(theRec->find(strBusy_period, gi));
	Assert(gi->sym());
	return &(GoalSym&)gi->sym()->cast(GoalSym::TheType);
}

RndDistr *SessionSym::idlePeriodDuration() const {
	return getDistr(strIdle_period_duration);
}

bool SessionSym::heartbeatGap(Time &gap) const {
	double rate;
	if (getRate(strHeartbeat_notif_rate, rate)) {
		gap = rate > 0 ? Time::Secd(1/rate) : Time();
		return true;
	}
	return false;
}
