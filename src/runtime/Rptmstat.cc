
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "pgl/RptmstatSym.h"
#include "runtime/StatPhase.h"
#include "runtime/Rptmstat.h"
#include "runtime/Xaction.h"
#include "runtime/LogComment.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "runtime/ErrorMgr.h"


bool Rptmstat::IsEnabled = false;


Rptmstat::Rptmstat(): theLoadDelta(-1.0), thePhase(0) {
	theChannels.append(TheXactEndChannel);
}

void Rptmstat::configure(const RptmstatSym &cfg) {
	theSampleDur = cfg.sampleDur();
	theRptmMin = cfg.rptmMin();
	theRptmMax = cfg.rptmMax();
	
	if (!cfg.loadDelta(theLoadDelta) || !theLoadDelta)
		Comment(0) << cfg.loc() 
			<< "must specify non-zero load delta for rptmstat" << endc << xexit;

	if (theSampleDur < 0)
		Comment(0) << cfg.loc()
			<< "must specify the sample duration for rptmstat" << endc << xexit;

	if (theRptmMin < 0 && theRptmMax < 0)
		Comment(0) << cfg.loc()
			<< "must specify at least one rptm bound for rptmstat" << endc << xexit;

	if (Time(0,0) <= theRptmMax && theRptmMax < theRptmMin)
		Comment(0) << cfg.loc() << "rptm_min must not exceed rptm_max" << endc << xexit;
}

void Rptmstat::start(StatPhase *aPhase) {
	Assert(!thePhase && aPhase);
	thePhase = aPhase;
	if (IsEnabled) {
		Comment(6) << "fyi: rptmstat is on;"
			<< " rptm goal: [" << theRptmMin << ", " << theRptmMax << "] " 
			<< " sample_dur: " << theSampleDur
			<< " load factor: " << (100*theLoadDelta) << '%'
			<< endc;
		startListen();
		nextSample();
	}
}

void Rptmstat::stop(StatPhase *aPhase) {
	Assert(thePhase && aPhase == thePhase);
	if (IsEnabled) {
		stopListen();
		Comment(6) << "fyi: rptmstat is off" << endc;
	}
	thePhase = 0;
	cancelAlarms();
}

void Rptmstat::nextSample() {
	theRptm.reset();
	sleepFor(theSampleDur);
}

void Rptmstat::wakeUp(const Alarm &alarm) {
	AlarmUser::wakeUp(alarm);
	checkpoint();
	nextSample();
}

void Rptmstat::checkpoint() {
	Assert(IsEnabled && thePhase);
	if (theRptm.count() && theRptm.mean() >= 0) {
		const Time rptm = Time::Msec((int)theRptm.mean());
		if (rptm >= 0 && rptm < theRptmMin)
			changeLoad(+theLoadDelta);
		else
		if (theRptmMax >= 0 && theRptmMax < rptm)
			changeLoad(-theLoadDelta);
		else
			Comment(8) << "fyi: rptmstat: " << rptm << " rptm requires no load adjustment" << endc;
	} else {
		Comment(7) << "fyi: rptmstat: got no usable measurements" << endc;
	}
}

void Rptmstat::changeLoad(double delta) {
	thePhase->loadFactor().changeBy(delta);
	ostream &os = Comment(7);
	const ios_fmtflags flags = os.flags();
	os << "fyi: rptmstat: " << Time::Msec((int)theRptm.mean())
		<< " rptm changes load by " << setiosflags(ios::showpos) 
		<< (100*delta)	<< "% current factor level: "
		<< thePhase->loadFactor().current();
	os.flags(flags);
	os << endc;
}

void Rptmstat::noteXactEvent(BcastChannel *ch, const Xaction *x) {
	if (ch == TheXactEndChannel)
		theRptm.record(x->lifeTime().msec());
}
