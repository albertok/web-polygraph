
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_LEVELTRACEFIG_H
#define POLYGRAPH__LOGANALYZERS_LEVELTRACEFIG_H

#include "xstd/String.h"
#include "loganalyzers/ReportTraceFigure.h"

class LevelStex;
class PhaseInfo;
class PhaseTrace;
class StatIntvlRec;

// creates level trace figure based on interval level stats
class LevelTraceFig: public ReportTraceFigure {
	public:
		typedef LevelStex Stex;

	public:
		LevelTraceFig();

		void stats(const LevelStex *const aStex, const PhaseInfo *const phase);
		void compareWith(const LevelStex *const stex);

	protected:
		virtual int addPlotData();

		int dumpDataLines(const LevelStex *stex);
		int dumpDataLine(const LevelStex *stex, Time stamp, const StatIntvlRec &r);
		
	protected:
		const LevelStex *theStex;
		Array<const LevelStex *> theComparison; // theStex and others
		const PhaseInfo *thePhase;
		const PhaseTrace *theTrace;
};

#endif
