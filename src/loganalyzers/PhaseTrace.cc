
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "loganalyzers/PhaseTrace.h"

Time PhaseTrace::TheWinLen = Time::Sec(60);


bool PhaseTraceWin::contains(Time tm) const {
	return start <= tm && tm < start + PhaseTrace::TheWinLen;
}


PhaseTrace::PhaseTrace() {
}

PhaseTrace::~PhaseTrace() {
	while (theWins.count()) delete theWins.pop().stats;
}

void PhaseTrace::configure(const StatIntvlRec &stats) {
	theAggr.merge(stats);
	theWins.stretch((int)(stats.theDuration/TheWinLen));
}

Time PhaseTrace::start() const {
	return theWins.count() ? theWins[0].start : Time();
}

Time PhaseTrace::winPos(int idx) const {
	Assert(0 <= idx && idx < theWins.count());
	return theWins[idx].start + TheWinLen/2;
}

const StatIntvlRec &PhaseTrace::winStats(int idx) const {
	Assert(0 <= idx && idx < theWins.count());
	Assert(theWins[idx].stats);
	return *theWins[idx].stats;
}

void PhaseTrace::addIntvl(Time tm, const StatIntvlRec &r) {
	PhaseTraceWin &w = allocWin(tm);
	w.stats->concat(r);
}

void PhaseTrace::mergeWin(const PhaseTraceWin &win) {
	PhaseTraceWin &w = allocWin(win.start);
	w.stats->merge(*win.stats);
}

void PhaseTrace::concatWin(const PhaseTraceWin &win) {
	PhaseTraceWin &w = allocWin(win.start);
	w.stats->concat(*win.stats);
}

PhaseTraceWin &PhaseTrace::allocWin(Time tm) {
	int idx = -1;
	if (!findWin(tm, idx)) {
		PhaseTraceWin win;
		win.stats = new StatIntvlRec;
		win.start = TheWinLen * (int)(tm/TheWinLen);
		theWins.insert(win, idx);
	}
	Assert(0 <= idx && idx < count());
	Assert(theWins[idx].stats);
	return theWins[idx];
}

// if tm belongs to a window, return that window's index
// otherwise, return index where a new window should be insterted to
// maintain win.start order
bool PhaseTrace::findWin(Time tm, int &idx) const {
	idx = 0;
	for (int left = 0, right = count() - 1; left <= right;) {
		idx = (left + right)/2;

		if (theWins[idx].contains(tm))
			return true;
		else
		if (theWins[idx].start < tm)
			left = ++idx;
		else
			right = idx-1;
	}
	return false;
}

void PhaseTrace::merge(const PhaseTrace &trace) {
	theAggr.merge(trace.theAggr);
	theWins.stretch(trace.theWins.count());

	for (int i = 0; i < trace.theWins.count(); ++i)
		mergeWin(trace.theWins[i]);
}

void PhaseTrace::concat(const PhaseTrace &trace) {
	theAggr.concat(trace.theAggr);
	theWins.stretch(theWins.count() + trace.theWins.count());

	for (int i = 0; i < trace.theWins.count(); ++i)
		concatWin(trace.theWins[i]);
}

