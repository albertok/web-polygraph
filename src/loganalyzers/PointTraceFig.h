
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_POINTTRACEFIG_H
#define POLYGRAPH__LOGANALYZERS_POINTTRACEFIG_H

#include "xstd/String.h"
#include "loganalyzers/ReportTraceFigure.h"

class PointStex;
class PhaseInfo;
class PhaseTrace;
class StatIntvlRec;

// creates a trace figure based on values extracted by supplied PointStexes
class PointTraceFig: public ReportTraceFigure {
	public:
		typedef PointStex Stex;

	public:
		PointTraceFig();

		void stats(const Stex *aStex1, const Stex *aStex2, const PhaseInfo *phase);

	protected:
		virtual int addPlotData();

		bool hasDataLines(const PointStex *stex) const;
		int dumpDataLines(const PointStex *stex);
		int dumpDataLine(const PointStex *stex, Time stamp, const StatIntvlRec &r);
		
	protected:
		const Stex *theStex1;
		const Stex *theStex2;
		const PhaseInfo *thePhase;
		const PhaseTrace *theTrace;
};

#endif
