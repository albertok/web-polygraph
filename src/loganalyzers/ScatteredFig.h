
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_SCATTEREDFIG_H
#define POLYGRAPH__LOGANALYZERS_SCATTEREDFIG_H

#include "xstd/String.h"
#include "loganalyzers/ReportFigure.h"

class PointStex;
class PhaseInfo;
class PhaseTrace;
class StatIntvlRec;

// scattered plot based on trace info
class ScatteredFig: public ReportFigure {
	public:
		typedef PointStex Stex;

	public:
		ScatteredFig();

		void stats(const Stex *aStex1, const Stex *aStex2, const PhaseInfo *phase);

	protected:
		virtual int addPlotData();

		int dumpDataLine(const StatIntvlRec &r);
		
	protected:
		const Stex *theStex1;
		const Stex *theStex2;
		const PhaseInfo *thePhase;
		const PhaseTrace *theTrace;
};

#endif

