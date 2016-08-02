
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/LogComment.h"
#include "pgl/RobotSym.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "client/AsyncClt.h"
#include "xstd/Rnd.h"

AsyncClt::AsyncClt(RndDistr *aThinkDistr) {
	Assert(!theThinkDistr);
	Assert(aThinkDistr);
	theThinkDistr = aThinkDistr;
}

void AsyncClt::configure(const RobotSym *cfg, const NetAddr &aHost) {
	Assert(theThinkDistr);
	Client::configure(cfg, aHost);
}

void AsyncClt::becomeIdle() {
	cancelAlarms(); // unschedule the next request if any
	Client::becomeIdle();
}

void AsyncClt::wakeUp(const Alarm &a) {
	Client::wakeUp(a);
	scheduleLaunch(a.time()); // next
	tryLaunch();
}

void AsyncClt::scheduleLaunch(Time lastLaunch) {
	if (isIdle)
		return;

	Assert(!thePendAlarmCnt);
	Assert(theThinkDistr);

	const double lf = TheStatPhaseMgr->loadFactor().current();

	if (lf <= 0) // zero load: no more requests needed
		return;

	double delay;
	if (TheStatPhaseMgr->loadFactor().flat()) {
		delay = theThinkDistr->trial()/lf;
	} else {
		// next launch should happen [0, 2dt] from last launch
		// where dt is 1/current_rate
		static RndGen rng;
		delay = rng(0.0, 2.*theThinkDistr->mean()/lf);
	}
	sleepTill(lastLaunch + Time::Secd(delay));
}

bool AsyncClt::launchCanceled(CltXact *x) {
	// if no alarms are pending, we may get stuck and submit
	// no more xactions (but this should not happen with AsyncClt)
	Should(isIdle || thePendAlarmCnt || theCcXactLvl);
	return Client::launchCanceled(x);
}
