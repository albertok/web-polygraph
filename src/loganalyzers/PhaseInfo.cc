
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "loganalyzers/PhaseTrace.h"
#include "loganalyzers/PhaseInfo.h"

PhaseInfo::PhaseInfo(): theIntvlCount(0), theTrace(0), gotPhaseStats(false) {
}

PhaseInfo::~PhaseInfo() {
	delete theTrace;
}

const String PhaseInfo::name() const {
	return thePhase.theName;
}

const StatPhaseRec *PhaseInfo::hasStats() const {
	return gotPhaseStats ? &thePhase : 0;
}

const StatIntvlRec &PhaseInfo::availStats() const {
	Assert(theIntvl.theDuration >= 0);
	return hasStats() ? (const StatIntvlRec &)stats() : theIntvl;
}

void PhaseInfo::merge(const PhaseInfo &phase) {
	if (!thePhase.theName)
		thePhase.theName = phase.thePhase.theName;

	if (phase.hasStats()) {
		thePhase.merge(phase.stats());
		gotPhaseStats = true;
	}
	theIntvl.merge(phase.theIntvl);
	theIntvlCount = Max(theIntvlCount, phase.theIntvlCount);

	Assert(phase.theTrace);
	if (!theTrace) {
		theTrace = new PhaseTrace;
		theTrace->concat(phase.trace());
	} else {
		theTrace->merge(phase.trace());
	}
}

void PhaseInfo::concat(const PhaseInfo &phase) {
	if (!thePhase.theName)
		thePhase.theName = phase.thePhase.theName;

	if (phase.hasStats()) {
		thePhase.concat(phase.stats());
		gotPhaseStats = true;
	}
	theIntvl.concat(phase.theIntvl);
	theIntvlCount = theIntvlCount + phase.theIntvlCount;

	Assert(phase.theTrace);
	if (!theTrace)
		theTrace = new PhaseTrace;
	theTrace->concat(phase.trace());
}

PhaseTrace *PhaseInfo::startTrace() {
	theTrace = new PhaseTrace;
	theTrace->configure(theIntvl);
	return theTrace;
}

void PhaseInfo::noteIntvl(const StatIntvlRec &r, const String &phaseName) {
	Assert(!theTrace);

	if (thePhase.theName)
		Should(thePhase.theName == phaseName);
	else
		thePhase.theName = phaseName;

	theIntvl.concat(r);
	theIntvlCount++;
}

void PhaseInfo::noteEndOfLog() {
	Assert(!theTrace);
	if (thePhase.theDuration < 0)
		((StatIntvlRec&)thePhase).merge(theIntvl); // restore what we can
}

void PhaseInfo::notePhase(const StatPhaseRec &r) {
	Assert(!theTrace);
	Assert(!thePhase.name() || thePhase.name() == r.name());
	Assert(thePhase.theDuration < 0);
	thePhase.concat(r);
	gotPhaseStats = true;
}

void PhaseInfo::checkConsistency() {
	if (!theIntvlCount)
		cerr << thePhase.theName << ": strange, no stat intervals within a phase" << endl;
}

void PhaseInfo::compileStats(BlobDb &) {
}
