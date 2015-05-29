
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
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"

StatPhaseMgr TheStatPhaseMgr;


StatPhaseMgr::StatPhaseMgr(): thePhase(0), thePhaseIdx(-1), theLogCat(lgcAll) {
}

StatPhaseMgr::~StatPhaseMgr() {
	if (thePhaseIdx >= thePhases.count())
		delete thePhase;
	thePhase = 0;
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

void StatPhaseMgr::nextPhase() {
	const StatPhase *prevPhase = thePhase;

	const bool mustStop(thePhase && thePhase->mustStop());
	if (mustStop ||
		thePhaseIdx >= thePhases.count() - 1) {
		// all phases have finished or negative goal was reached
		ostringstream s;
		if (mustStop) {
			thePhaseIdx = thePhases.count();
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
		// note: this phase is never deleted, but that's OK
		thePhase = new StatPhase;
		thePhase->name("cold");
		thePhase->statsLogged(false);
	} else {
		thePhase = thePhases[++thePhaseIdx];
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
