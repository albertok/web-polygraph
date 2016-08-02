
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_HISTOGRAMFIGURE_H
#define POLYGRAPH__LOGANALYZERS_HISTOGRAMFIGURE_H

#include "loganalyzers/ReportFigure.h"

class HistStex;
class Histogram;
class HistogramBin;
class PhaseInfo;

// creates a distribution figure based on phase stats
class HistogramFigure: public ReportFigure {
	public:
		HistogramFigure();

		void stats(const HistStex *aStex, const PhaseInfo *phase);
		void compareWith(const HistStex *stex);

	protected:
		virtual int addPlotData();
		int dumpDataLine(const HistogramBin &bin, const Counter totCount);
		
	protected:
		const PhaseInfo *thePhase;
		const HistStex *theStex;
		Array<const HistStex *> theComparison; // theStex and others
};

#endif
