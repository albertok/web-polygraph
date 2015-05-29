
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_TMSZHISTFIG_H
#define POLYGRAPH__LOGANALYZERS_TMSZHISTFIG_H

#include "loganalyzers/HistogramFigure.h"

class TmSzHistStat;
class Stex;

// creates response time or response size distribution figure
// based on phase stats
class TmSzHistFig: public HistogramFigure {
	public:
		TmSzHistFig(const String &aUnit);
		virtual ~TmSzHistFig();

		void stats(const Stex *aStex, const PhaseInfo *phase);

	protected:
		virtual const Histogram *extractHist(const Stex *stex, const PhaseInfo &phase) const = 0;

	protected:
		String theUnit;
		Stex *theAuth;
};

#endif
