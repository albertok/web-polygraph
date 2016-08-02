
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/gadgets.h"
#include "base/polyLogTags.h"
#include "base/polyLogCats.h"
#include "pgl/StatsSampleSym.h"
#include "pgl/StatsSampleCfg.h"
#include "runtime/StatsSampleMgr.h"
#include "runtime/Xaction.h"
#include "runtime/PolyOLog.h"
#include "runtime/LogComment.h"

StatsSampleMgr TheStatsSampleMgr;


StatsSampleMgr::StatsSampleMgr():
	theStartCnt(0),
	thePendCfgCnt(0),
	didStart(false) {
}

StatsSampleMgr::~StatsSampleMgr() {
	while (theCfgs.count()) delete theCfgs.pop();
}

void StatsSampleMgr::configure(const Array<StatsSampleSym*> &cfgs) {
	theCfgs.stretch(cfgs.count());
	for (int i = 0; i < cfgs.count(); ++i) {
		StatsSampleCfg cfg;
		cfg.name = cfgs[i]->name();
		cfg.start = cfgs[i]->start();
		cfgs[i]->capacity(cfg.capacity);

		addSample(cfg);
	}
}

void StatsSampleMgr::addSample(const StatsSampleCfg &cfg) {
	if (cfg.start < 0 || cfg.capacity <= 0) {
		cerr << "malformed StatsSample entry ";
		if (cfg.name)
			cerr << " named " << cfg.name;
		cerr << endl;
		exit(-2);
	}

	// find the right position
	int idx = theStartCnt;
	while (idx < theCfgs.count() && theCfgs[idx]->start <= cfg.start)
		idx++;

	// move later samples up
	for (int i = theCfgs.count(); i > idx; --i) {
		StatsSampleCfg * const tmp = theCfgs[i-1];
		theCfgs.put(tmp, i);
	}

	theCfgs.put(new StatsSampleCfg(cfg), idx);

	if (didStart)
		schedSample(cfg);
}

void StatsSampleMgr::willAddSample() {
	thePendCfgCnt++;
}

void StatsSampleMgr::start() {
	if (!didStart) {
		didStart = true;
		const Time waitTime = TheClock.time() - Clock::TheStartTime;
		Must(waitTime >= 0);
		for (int i = 0; i < theCfgs.count(); ++i) {
			theCfgs[i]->start += waitTime;
			schedSample(*theCfgs[i]);
		}
	}
}

void StatsSampleMgr::wakeUp(const Alarm &a) {
	AlarmUser::wakeUp(a);
	Must(didStart); // we should not have been sleeping otherwise
	const Time t = Max(a.time(), TheClock.time());
	while (theStartCnt < theCfgs.count() &&
		theCfgs[theStartCnt]->start + Clock::TheStartTime <= t) {
		startSample();
	}
}

void StatsSampleMgr::startSample() {
	Assert(theStartCnt < theCfgs.count());

	if (Xaction::TheSampleDebt > 0) {
		Comment(3) << "warning: stats samples overlap; "
			<< Xaction::TheSampleDebt << " entries still uncollected" 
			<< endc;
	}
	const StatsSampleCfg &cfg = *theCfgs[theStartCnt];
	Xaction::TheSampleDebt += cfg.capacity;

	Comment(3) << "fyi: started sampling ";
	if (cfg.name)
		Comment << "`" << cfg.name << "' ";
	Comment << '(' << cfg.capacity << " entries)";
	Comment << endc;

	theStartCnt++;

	(*TheSmplOLog) << bege(lgStatsSmplBeg, lgcAll)
		<< cfg.name << cfg.start << cfg.capacity << ende;
}

void StatsSampleMgr::schedSample(const StatsSampleCfg &cfg) {
	sleepTill(Clock::TheStartTime + cfg.start);
}

void StatsSampleMgr::reportCfg(ostream &os) const {
	os << "\tstatic stats samples:  " << setw(2) << thePendCfgCnt << endl;
	os << "\tdynamic stats samples: " << setw(2) << theCfgs.count() << endl;

	if (theCfgs.count()) {
		// header
		os << "\t"
			<< setw(16) << "sample"
			<< ' ' << setw(8) << "capacity"
			<< ' ' << setw(8) << "start"
			<< endl;

		for (int i = 0; i < theCfgs.count(); ++i) {
			const StatsSampleCfg &cfg = *theCfgs[i];
			os << "\t"
				<< setw(16) << (cfg.name ? cfg.name.cstr() : "<anonymous>")
				<< ' ' << setw(8) << cfg.capacity
				<< ' ' << setw(8) << cfg.start
				<< endl;
		}
	}
}
