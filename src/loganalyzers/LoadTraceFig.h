
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_LOADTRACEFIG_H
#define POLYGRAPH__LOGANALYZERS_LOADTRACEFIG_H

#include "xstd/String.h"
#include "loganalyzers/ReportTraceFigure.h"

class LoadStex;
class PhaseInfo;
class PhaseTrace;
class StatIntvlRec;

// creates load trace figure based on interval load stats
class LoadTraceFig: public ReportTraceFigure {
	public:
		typedef LoadStex Stex;

	public:
		LoadTraceFig();

		void stats(const Stex *aStex, const PhaseInfo *phase);
		void compareWith(const Stex *stex);

	protected:
		virtual int addPlotData();

		enum lineType { lnRate, lnBwidth };
		int dumpDataLines(const LoadStex *stex, const lineType lt);
		int dumpDataLine(const LoadStex *stex, Time stamp, const StatIntvlRec &r, const lineType lt);
		
	protected:
		const Stex *theStex;
		Array<const Stex *> theComparison; // theStex and others
		const PhaseInfo *thePhase;
		const PhaseTrace *theTrace;
};

#endif
