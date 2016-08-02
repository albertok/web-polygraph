
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/Histogram.h"
#include "loganalyzers/Stex.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/HistStex.h"
#include "loganalyzers/TmSzHistFig.h"

// converts Stex to HistStex
class TmSzHistStex: public HistStex {
	public:
		TmSzHistStex(const Stex *stex, const String aUnit, const Histogram *aHist): 
			HistStex(stex->key(), stex->name(), aUnit), theHist(aHist) {}

		virtual const Histogram *value(const PhaseInfo &) const {
			return theHist;
		}

	protected:
		const Histogram *theHist; // authoritative source of information
};


TmSzHistFig::TmSzHistFig(const String &aUnit): theUnit(aUnit) {
	theAxisX1.label(theUnit);
}

TmSzHistFig::~TmSzHistFig() {
	while (theComparison.count() > 0)
		delete theComparison.pop();
	theStex = 0;
}

void TmSzHistFig::stats(const Stex *aStex, const PhaseInfo *aPhase) {
	HistStex *core = new TmSzHistStex(aStex, theUnit, extractHist(aStex, *aPhase));
	HistogramFigure::stats(core, aPhase);

	// collect comparison stexes to plot
	for (const Stex *stex = aStex->parent(); stex; stex = stex->parent()) {
		compareWith(new TmSzHistStex(stex, theUnit, extractHist(stex, *aPhase)));
	}
}

