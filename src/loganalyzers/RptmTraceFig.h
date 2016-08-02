
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_RPTMTRACEFIG_H
#define POLYGRAPH__LOGANALYZERS_RPTMTRACEFIG_H

#include "xstd/String.h"
#include "loganalyzers/ReportTraceFigure.h"

class Stex;
class PhaseInfo;
class PhaseTrace;
class StatIntvlRec;

// creates response time trace figure based on interval stats
class RptmTraceFig: public ReportTraceFigure {
	public:
		RptmTraceFig();

		void stats(const Stex *aStex, const PhaseInfo *phase);
		void moreStats(const Stex *aStex);

	protected:
		virtual int addPlotData();
		int dumpDataLines(const Stex *stex);
		int dumpDataLine(const Stex *stex, Time stamp, const StatIntvlRec &r);
		
	protected:
		Array<const Stex*> theStexes;
		const PhaseInfo *thePhase;
		const PhaseTrace *theTrace;
};

#endif
