
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "base/polyLogCats.h"
#include "loganalyzers/InfoScope.h"
#include "loganalyzers/SideInfo.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/ProcInfo.h"

ProcInfo::ProcInfo(const String &aName): theName(aName), theSide(0), theLogCat(lgcEnd) {
}

ProcInfo::~ProcInfo() {
	while (thePhases.count()) delete thePhases.pop();
}

const String &ProcInfo::name() const {
	if (theName)
		return theName;

	static String defName = "unnamed";
	return defName;
}

int ProcInfo::logCat() const {
	return theLogCat;
}

const String &ProcInfo::benchmarkVersion() const {
	return theBenchmarkVersion;
}

const String &ProcInfo::pglCfg() const {
	return thePglCfg;
}

Time ProcInfo::startTime() const {
	return theStartTime;
}

void ProcInfo::logCat(int aLogCat) {
	Assert(theLogCat == lgcEnd && 0 <= aLogCat && aLogCat < lgcEnd);
	theLogCat = aLogCat;
}

void ProcInfo::side(SideInfo *aSide) {
	Assert(!theSide ^ !aSide);
	theSide = aSide;
}

void ProcInfo::benchmarkVersion(const String &aVersion) {
	Assert(!theBenchmarkVersion);
	theBenchmarkVersion = aVersion;
}

void ProcInfo::pglCfg(const String &aPglCfg) {
	Assert(!thePglCfg);
	thePglCfg = aPglCfg;
}

void ProcInfo::startTime(Time aStartTime) {
	Assert(theStartTime < 0);
	theStartTime = aStartTime;
}

const PhaseInfo &ProcInfo::execScopePhase() const {
	return theExecScopePhase;
}

const PhaseInfo &ProcInfo::allPhasesPhase() const {
	return theAllPhasesPhase;
}

Counter ProcInfo::repCount(const Scope &scope) const {
	Counter count = 0;
	for (int i = 0; i < thePhases.count(); ++i) {
		if (scope.hasPhase(thePhases[i]->name()))
			count += thePhases[i]->stats().theXactCnt; // all successfull
	}
	return count;
}

Counter ProcInfo::hitCount(const Scope &scope) const {
	Counter count = 0;
	for (int i = 0; i < thePhases.count(); ++i) {
		if (scope.hasPhase(thePhases[i]->name()))
			count += thePhases[i]->stats().theBasicXacts.hits().size().stats().count();
	}
	return count;
}

Counter ProcInfo::offeredHitCount(const Scope &scope) const {
	Counter count = 0;
	for (int i = 0; i < thePhases.count(); ++i) {
		if (scope.hasPhase(thePhases[i]->name()))
			count += thePhases[i]->stats().theIdealHR.hits().count();
	}
	return count;
}

Counter ProcInfo::uselessProxyValidationCount(const Scope &scope) const {
	Counter count = 0;
	for (int i = 0; i < thePhases.count(); ++i) {
		if (scope.hasPhase(thePhases[i]->name()))
			count += thePhases[i]->stats().theProxyValidationR.misses().aggr().count();
	}
	return count;
}

BigSize ProcInfo::repVolume(const Scope &scope) const {
	BigSize volume = 0;
	for (int i = 0; i < thePhases.count(); ++i) {
		if (scope.hasPhase(thePhases[i]->name())) {
			const TmSzStat repAll = thePhases[i]->stats().reps();
			volume += BigSize::Byted(repAll.size().sum());
		}
	}
	return volume;
}

BigSize ProcInfo::hitVolume(const Scope &scope) const {
	BigSize volume = 0;
	for (int i = 0; i < thePhases.count(); ++i) {
		if (scope.hasPhase(thePhases[i]->name()))
			volume += BigSize::Byted(thePhases[i]->stats().theBasicXacts.hits().size().stats().sum());
	}
	return volume;
}

BigSize ProcInfo::offeredHitVolume(const Scope &scope) const {
	BigSize volume = 0;
	for (int i = 0; i < thePhases.count(); ++i) {
		if (scope.hasPhase(thePhases[i]->name()))
			volume += BigSize::Byted(thePhases[i]->stats().theIdealHR.hits().size().sum());
	}
	return volume;
}

BigSize ProcInfo::uselessProxyValidationVolume(const Scope &scope) const {
	BigSize volume = 0;
	for (int i = 0; i < thePhases.count(); ++i) {
		if (scope.hasPhase(thePhases[i]->name()))
			volume += BigSize::Byted(thePhases[i]->stats().theProxyValidationR.misses().aggr().size().sum());
	}
	return volume;
}

AggrStat ProcInfo::lastReqByteWritten(const Scope &scope) const {
	AggrStat stat;
	for (int i = 0; i < thePhases.count(); ++i)
		if (scope.hasPhase(thePhases[i]->name()))
			stat += thePhases[i]->stats().theLastReqByteWritten.stats();
	return stat;
}

AggrStat ProcInfo::lastReqByteRead(const Scope &scope) const {
	AggrStat stat;
	for (int i = 0; i < thePhases.count(); ++i)
		if (scope.hasPhase(thePhases[i]->name()))
			stat += thePhases[i]->stats().theLastReqByteRead.stats();
	return stat;
}

AggrStat ProcInfo::firstRespByteWritten(const Scope &scope) const {
	AggrStat stat;
	for (int i = 0; i < thePhases.count(); ++i)
		if (scope.hasPhase(thePhases[i]->name()))
			stat += thePhases[i]->stats().theFirstRespByteWritten.stats();
	return stat;
}

AggrStat ProcInfo::firstRespByteRead(const Scope &scope) const {
	AggrStat stat;
	for (int i = 0; i < thePhases.count(); ++i)
		if (scope.hasPhase(thePhases[i]->name()))
			stat += thePhases[i]->stats().theFirstRespByteRead.stats();
	return stat;
}

void ProcInfo::noteIntvl(const StatIntvlRec &r, const String &phaseName) {
	if (!thePhases.count() || thePhases.last()->name() != phaseName)
		thePhases.append(new PhaseInfo());
	thePhases.last()->noteIntvl(r, phaseName);
}

void ProcInfo::addPhase(const StatPhaseRec &r) {
	int foundCount = 0;
	for (int i = 0; i < thePhases.count(); ++i) {
		PhaseInfo &pi = *thePhases[i];
		if (pi.name() == r.name()) {
			++foundCount;
			if (!pi.hasStats()) {
				pi.notePhase(r);
				break;
			}
		}
	}

	if (foundCount == 0) {
		clog << name() << ": strange, phase '" << r.name() << "' has "
			<< "phase statistics but no interval stats" << endl;
		thePhases.append(new PhaseInfo());
		thePhases.last()->noteIntvl(r, r.name());
		thePhases.last()->notePhase(r);
	} else
	if (foundCount > 1) {
		clog << name() << ": error: found " << foundCount <<
			" phases named '" << r.name() << "', even after trying " <<
			"to make all phase names unique; the reporter may fail or " <<
			"mislead" << endl;
	}
}

void ProcInfo::noteEndOfLog() {
	for (int i = 0; i < thePhases.count(); ++i)
		thePhases[i]->noteEndOfLog();
}

const PhaseInfo &ProcInfo::phase(int idx) const {
	Assert(0 <= idx && idx < thePhases.count());
	return *thePhases[idx];
}

const PhaseInfo *ProcInfo::hasPhase(const String &name) const {
	for (int i = 0; i < phaseCount(); ++i) {
		if (thePhases[i]->name() == name)
			return thePhases[i];
	}
	return 0;
}

const PhaseInfo &ProcInfo::phase(const String &name) const {
	const PhaseInfo *p = hasPhase(name);
	Assert(p);
	return *p;
}

int ProcInfo::phaseCount() const {
	return thePhases.count();
}

PhaseTrace *ProcInfo::tracePhase(const String &name) {
	for (int i = 0; i < phaseCount(); ++i) {
		if (thePhases[i]->name() == name)
			return thePhases[i]->startTrace();
	}
	return 0;
}

void ProcInfo::checkConsistency() {
	if (!theBenchmarkVersion)
		cerr << name() << ": strange, no benchmark version found" << endl;

	if (!thePglCfg)
		cerr << name() << ": strange, no PGL configuration found" << endl;

	if (theStartTime < 0)
		cerr << name() << ": strange, no startup time could be determined" << endl;
}

void ProcInfo::compileStats(BlobDb &) {
	const Scope &scope = theSide->execScope();
	for (int i = 0; i < phaseCount(); ++i) {
		if (scope.hasPhase(thePhases[i]->name()))
			theExecScopePhase.concat(*thePhases[i]);
		theAllPhasesPhase.concat(*thePhases[i]);
	}
}
