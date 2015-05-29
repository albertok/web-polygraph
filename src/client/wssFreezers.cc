
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/LogComment.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "client/wssFreezers.h"


/* TimeWssFreezer */

TimeWssFreezer::TimeWssFreezer(Time aDelay): theDelay(aDelay) {
}

void TimeWssFreezer::start() {
	WssFreezer::start();
	sleepFor(theDelay);
	theStartTime = TheClock;
}

void TimeWssFreezer::wakeUp(const Alarm &alarm) {
	AlarmUser::wakeUp(alarm);
	startFreeze();
}

void TimeWssFreezer::noteInfoEvent(BcastChannel *ch, InfoEvent ev) {
	Assert(ch == TheInfoChannel);
	if (ev == ieReportProgress) {
		const Time duration(TheClock - theStartTime);
		Comment(7) << "fyi:"
			<< " fill duration: " << duration
			<< " working set fill duration goal: " << theDelay
			<< " (" << Percent(duration.secd(), theDelay.secd())
			<< "% complete)" << endl
			<< endc;
	}
}

void TimeWssFreezer::printGoal(ostream &os) const {
	os << "first " << theDelay << " of the test";
}


/* FillWssFreezer */

FillWssFreezer::FillWssFreezer(int aFillCount): theFillCount(aFillCount) {
	theChannels.append(TheXactEndChannel);
}

void FillWssFreezer::noteXactEvent(BcastChannel *ch, const Xaction *) {
	if (ch == TheXactEndChannel && TheStatPhaseMgr->fillCnt() >= theFillCount) {
		startFreeze();
	}
}

void FillWssFreezer::noteInfoEvent(BcastChannel *ch, InfoEvent ev) {
	Assert(ch == TheInfoChannel);
	if (ev == ieReportProgress) {
		Comment(7) << "fyi:"
			<< " fill objects count: " << TheStatPhaseMgr->fillCnt() 
			<< " working set size goal: " << theFillCount
			<< " (" << Percent(TheStatPhaseMgr->fillCnt(), theFillCount)
			<< "% complete)" << endl 
			<< endc;
	}
}

void FillWssFreezer::printGoal(ostream &os) const {
	os << "first " << theFillCount << " fill objects of the test";
}
