
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "base/StatIntvlRec.h"
#include "base/polyLogCats.h"
#include "pgl/PglCodeSym.h"
#include "pgl/EveryCodeSym.h"
#include "runtime/Goal.h"
#include "runtime/Script.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/DutWatchdog.h"


bool DutWatchdog::IsEnabled = false;
int DutWatchdog::TheLastId = 0;


DutWatchdog::DutWatchdog(): theId(-1), theGoal(0), theScript(0), theStats(0) {
	theId = ++TheLastId;
	theStats = new StatIntvlRec;
}

DutWatchdog::~DutWatchdog() {
	delete theGoal;
	delete theScript;
	delete theStats;
}

void DutWatchdog::configure(const EveryCodeSym &cfg) {
	theGoal = new Goal;
	configureGoal(*cfg.goal());

	const CodeSym *codeCfg = cfg.call();
	Assert(codeCfg);
	theScript = new Script(*codeCfg);
	theScript->statsSampler(this);
}

// may be called run-time to reconfigure
void DutWatchdog::configureGoal(const GoalSym &goalCfg) {
	Assert(theGoal);
	theGoal->configure(goalCfg);
}

void DutWatchdog::start() {
	if (IsEnabled) {
		theStart = TheClock;

		Comment(6) << "fyi: DUT watchdog(" << theId << ") is on:" << endl;

		Comment << "\tsampling goal:" << endl;
		theGoal->print(Comment, "\t\t");

		Comment << "\tcode:" << endl << 
			"\t\t" << theScript->code() << endc;

		startListen();
		nextSample();
		checkpoint();
	}
}

void DutWatchdog::stop() {
	if (IsEnabled) {
		stopListen();
		Comment(6) << "fyi: DUT watchdog(" << theId << ") is off" << endc;
	}
	cancelAlarms();
}

void DutWatchdog::wakeUp(const Alarm &alarm) {
	AlarmUser::wakeUp(alarm);
	checkpoint();
}

StatIntvlRec &DutWatchdog::getRec(int cat) {
	// XXX: we should change getRec() to return pointers to indicate absence of
	// as record for a given log category
	Assert(cat == lgcCltSide);
	theStats->theDuration = duration();
	return *theStats;
}

const StatIntvlRec &DutWatchdog::getRec(int cat) const {
	Assert(cat == lgcCltSide);
	return *theStats;
}

bool DutWatchdog::checkpoint() {
	Assert(IsEnabled);

	theStats->theDuration = duration();

	if (!theGoal->reached(*this))
		return false;

	act();
	nextSample();
	return false;
}

void DutWatchdog::nextSample() {
	cancelAlarms();
	theStart = TheClock;
	theStats->restart();

	//theScript->reset();

	if (theGoal->duration() >= 0)
		sleepFor(theGoal->duration());
}

void DutWatchdog::act() {
	// Comment(6) << "fyi: DUT watchdog(" << theId << ") acts" << endc;
	// XXX: this should be an asynchronous call
	theScript->run();
}

Time DutWatchdog::duration() const {
	return TheClock - theStart;
}

Counter DutWatchdog::xactCnt() const {
	return theStats->xactCnt();
}

BigSize DutWatchdog::fillSz() const {
	return theStats->totFillSize();
}

Counter DutWatchdog::fillCnt() const {
	return theStats->totFillCount();
}

Counter DutWatchdog::xactErrCnt() const {
	return theStats->theXactErrCnt;
}
