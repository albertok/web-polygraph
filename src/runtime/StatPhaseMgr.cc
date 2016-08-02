
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"
#include "xstd/h/sstream.h"

#include "base/polyLogCats.h"
#include "runtime/ErrorMgr.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/StatsSampleMgr.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"

StatPhaseMgr TheStatPhaseMgr;


StatPhaseMgr::StatPhaseMgr():
	thePhase(0),
	theWaitPhase(0),
	thePhaseIdx(-1),
	theLogCat(lgcAll),
	reachedNegative_(false) {
}

StatPhaseMgr::~StatPhaseMgr() {
	delete theWaitPhase;
	theWaitPhase = 0;

	// cold phase
	if (thePhaseIdx >= thePhases.count())
		delete thePhase;
	thePhase = 0;

	// active phases
	while (thePhases.count()) delete thePhases.pop();
}

void StatPhaseMgr::addPhase(StatPhase *ph) {
	Assert(ph);
	Assert(!ph->used());
	thePhases.append(ph);
}

int StatPhaseMgr::phaseSyncPos() const {
	return thePhaseIdx + (thePhase->unlockToStop() ? 1 : 0);
}

void StatPhaseMgr::start() {
	nextPhase();
}

void StatPhaseMgr::forceFlush() {
	for (int i = 0; i < thePhases.count(); ++i)
		if (thePhases[i]->used())
			thePhases[i]->flush();
}

void StatPhaseMgr::noteDone(StatPhase *ph) {
	Assert(thePhase == ph);
	nextPhase();
}

void StatPhaseMgr::stopTrafficWaiting() {
	if (ShouldUs(trafficWaiting()) &&
		ShouldUs(thePhase) &&
		ShouldUs(thePhase == theWaitPhase))
		thePhase->stop(); // not report()ing the all-zeros "wait" phase

	nextPhase();
}

void StatPhaseMgr::nextPhase() {
	// do not treat theWaitPhase with its fake factors as a previous phase
	const StatPhase *prevPhase = thePhaseIdx >= 0 ? thePhase : 0;

	const bool mustStop(thePhase && thePhase->mustStop());
	if (mustStop ||
		thePhaseIdx >= thePhases.count() - 1) {
		// all phases have finished or negative goal was reached
		ostringstream s;
		if (mustStop) {
			thePhaseIdx = thePhases.count();
			reachedNegative_ = true;
			s << "phase '" << thePhase->name() << "' reached negative goal" << ends;
			ReportError(errNegativePhase);
		} else {
			++thePhaseIdx;
			s << "all " << thePhases.count() << " phases met their goals" << ends;
		}
		Broadcast(ThePhasesEndChannel, s.str().c_str());
		streamFreeze(s, false);

		// add a never-finishing phase when all official phases are over
		// so that agents do not have to check for some phase to exist
		thePhase = StatPhase::MakeVirtual("cold", prevPhase);
	} else if (!theWaitPhase && thePhaseIdx < 0) {
		thePhase = theWaitPhase = StatPhase::MakeVirtual("wait", prevPhase);
	} else {
		thePhase = thePhases[++thePhaseIdx];
		if (thePhaseIdx == 0)
			TheStatsSampleMgr.start();
	}

	Assert(thePhase);
	thePhase->start(this, prevPhase);
}

void StatPhaseMgr::reportCfg(ostream &os) const {

	// header
	os 
		<< setw(10) << "phase"
		<< ' ' << setw(8) << "pop_beg"
		<< ' ' << setw(8) << "pop_end"
		<< ' ' << setw(8) << "load_beg"
		<< ' ' << setw(8) << "load_end"
		<< ' ' << setw(8) << " rec_beg"
		<< ' ' << setw(8) << " rec_end"
		<< ' ' << setw(8) << "smsg_beg"
		<< ' ' << setw(8) << "smsg_end"
		<< ' ' << setw(8) << "goal"
		<< "\t flags"
		<< endl;

	for (int i = 0; i < thePhases.count(); ++i)
		thePhases[i]->reportCfg(os);
}
