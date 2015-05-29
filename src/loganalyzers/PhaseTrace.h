
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_PHASETRACE_H
#define POLYGRAPH__LOGANALYZERS_PHASETRACE_H

#include "base/StatIntvlRec.h"

class PhaseTraceWin {
	public:
		StatIntvlRec *stats;
		Time start;

		bool contains(Time tm) const;
};

// maintains trace data
class PhaseTrace {
	public:
		static Time TheWinLen;

	public:
		PhaseTrace();
		~PhaseTrace();

		void configure(const StatIntvlRec &stats);

		const StatIntvlRec &aggr() const { return theAggr; }

		int count() const { return theWins.count(); }
		const StatIntvlRec &winStats(int idx) const;
		Time winPos(int idx) const;
		Time start() const;

		void addIntvl(Time tm, const StatIntvlRec &r);
		void merge(const PhaseTrace &trace);
		void concat(const PhaseTrace &trace);

	protected:
		void mergeWin(const PhaseTraceWin &win);
		void concatWin(const PhaseTraceWin &win);
		PhaseTraceWin &allocWin(Time tm);
		bool findWin(Time tm, int &idx) const;

	protected:
		StatIntvlRec theAggr; // all intervals together
		Array<PhaseTraceWin> theWins;
};

#endif
