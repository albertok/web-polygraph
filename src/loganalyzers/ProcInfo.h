
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_PROCINFO_H
#define POLYGRAPH__LOGANALYZERS_PROCINFO_H

#include "xstd/Time.h"
#include "xstd/String.h"
#include "xstd/BigSize.h"
#include "xstd/Array.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/SomeInfo.h"

class SideInfo;
class PhaseInfo;
class PhaseTrace;
class BlobDb;

// aggregate stats and other logged information about a Polygraph process
class ProcInfo: public SomeInfo {
	public:
		ProcInfo(const String &name);
		~ProcInfo();

		const String &name() const;
		int logCat() const;
		const SideInfo *side() const;
		const String &benchmarkVersion() const;
		const String &pglCfg() const;
		Time startTime() const;
		const PhaseInfo &allPhasesPhase() const;
		const PhaseInfo &execScopePhase() const;

		// delete these, use allPhaseStats() ?
		Counter repCount(const Scope &scope) const;
		Counter hitCount(const Scope &scope) const;
		Counter offeredHitCount(const Scope &scope) const;
		Counter uselessProxyValidationCount(const Scope &scope) const;
		BigSize repVolume(const Scope &scope) const;
		BigSize hitVolume(const Scope &scope) const;
		BigSize offeredHitVolume(const Scope &scope) const;
		BigSize uselessProxyValidationVolume(const Scope &scope) const;
		AggrStat lastReqByteWritten(const Scope &scope) const;
		AggrStat lastReqByteRead(const Scope &scope) const;
		AggrStat firstRespByteWritten(const Scope &scope) const;
		AggrStat firstRespByteRead(const Scope &scope) const;

		void logCat(int aLogCat);
		void side(SideInfo *aSide);
		void benchmarkVersion(const String &aVersion);
		void pglCfg(const String &aPglCfg);
		void startTime(Time aStartTime);

		void noteIntvl(const StatIntvlRec &r, const String &phaseName);
		void addPhase(const StatPhaseRec &r);
		void noteEndOfLog();

		int phaseCount() const;
		const PhaseInfo &phase(int idx) const;
		const PhaseInfo &phase(const String &name) const;
		const PhaseInfo *hasPhase(const String &name) const;
		PhaseTrace *tracePhase(const String &name);

		void checkConsistency();
		void compileStats(BlobDb &db);

	protected:
		String theName;
		SideInfo *theSide;
		int theLogCat;
		String theBenchmarkVersion;
		String thePglCfg;
		Time theStartTime;

		Array<PhaseInfo*> thePhases;
		PhaseInfo theExecScopePhase;
		PhaseInfo theAllPhasesPhase;
};

#endif
