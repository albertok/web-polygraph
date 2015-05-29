
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_PHASEINFO_H
#define POLYGRAPH__LOGANALYZERS_PHASEINFO_H

#include "base/StatPhaseRec.h"
#include "loganalyzers/SomeInfo.h"

class PhaseTrace;

// information about a stats phase
class PhaseInfo: public SomeInfo {
	public:
		PhaseInfo();
		~PhaseInfo();

		const String name() const;
		const StatPhaseRec *hasStats() const;
		const StatPhaseRec &stats() const { return thePhase; }
		const StatIntvlRec &availStats() const;
		const PhaseTrace &trace() const { return *theTrace; }

		void concat(const PhaseInfo &phase);
		void merge(const PhaseInfo &phase);

		PhaseTrace *startTrace();

		void noteIntvl(const StatIntvlRec &r, const String &phaseName);
		void notePhase(const StatPhaseRec &r);
		void noteEndOfLog();

		void checkConsistency();
		void compileStats(BlobDb &db);

	protected:
		StatPhaseRec thePhase;
		StatIntvlRec theIntvl; // all intervals together
		int theIntvlCount;     // number of intervals seen

		PhaseTrace *theTrace;

		bool gotPhaseStats; // and not just recovered interval-based ones
};

#endif
