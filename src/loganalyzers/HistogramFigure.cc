
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"

#include "xstd/gadgets.h"
#include "base/Histogram.h"
#include "loganalyzers/HistStex.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/HistogramFigure.h"


HistogramFigure::HistogramFigure(): thePhase(0), theStex(0) {
}

void HistogramFigure::stats(const HistStex *aStex, const PhaseInfo *aPhase) {
	theStex = aStex;
	thePhase = aPhase;
}

void HistogramFigure::compareWith(const HistStex *stex) {
	Assert(thePhase);
	if (stex->value(*thePhase))
		theComparison.append(stex);
}

void HistogramFigure::setCtrlOptions() {
	theLabelX1 = theStex->unit();
	theLabelY1 = "cumulative %";
	ReportFigure::setCtrlOptions();
	*theCtrlFile << "set key right bottom" << endl;
}

int HistogramFigure::createCtrlFile() {
	if (ReportFigure::createCtrlFile() < 0)
		return -1;

	// make sure that the most interesting line is on top
	if (theStex->value(*thePhase))
		theComparison.append(theStex);

	// create plot command
	for (int i = 0; i < theComparison.count(); ++i)
		addPlotLine(theComparison[i]->name(), theLabelY1);
	addedAllPlotLines();

	// dump data to plot
	int pointCount = 0;
	for (int s = 0; s < theComparison.count(); ++s) {
		if (s)
			*theCtrlFile << 'e' << endl; // note: two empty lines do not work

		const Histogram *hist = theComparison[s]->value(*thePhase);
		const int count = hist->stats().count();
		int c = 0;
		for (HistogramConstIter i(*hist); count && i; ++i)
			c += dumpDataLine(*i, count);

		if (theStex == theComparison[s])
			pointCount = c;
	}

	return pointCount;
}

int HistogramFigure::dumpDataLine(const HistogramBin &bin, int totCount) {
	if (bin.count) {
		const double p = Percent(bin.accCount, totCount);

		if (p < 5)
			return 0; // ignore small values unless they contribute a lot

		if (Percent(bin.accCount-bin.count, totCount) > 95)
			return 0; // ingore large values unless they contribute a lot

		*theCtrlFile << bin.sup << ' ' << p << endl;
		return 1;
	}
	return 0;
}

