
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "client/SyncClt.h"


// note: only called after launch win delay and via launchCanceled()
void SyncClt::wakeUp(const Alarm &a) {
	Client::wakeUp(a);
	tryLaunch();
}

void SyncClt::scheduleLaunch(Time) {
	if (isIdle)
		return;

	Assert(!thePendAlarmCnt);
	Assert(!theThinkDistr);
	tryLaunch(genXact());
}

bool SyncClt::launchCanceled(CltXact *x) {
	const bool res = Client::launchCanceled(x);

	// if no alarms are pending, we may get stuck and submit
	// no more xactions; this is a kludge
	if (!isIdle && !thePendAlarmCnt && !theCcXactLvl)
		sleepTill(TheClock); // submit more next cycle

	return res;
}

void SyncClt::loneXactFollowup() {
	theExtraLaunchLvl = 0;
	tryLaunch();
}
