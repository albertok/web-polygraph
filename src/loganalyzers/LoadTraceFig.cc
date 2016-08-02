
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>

#include "xml/XmlAttr.h"
#include "loganalyzers/LoadStex.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/RepOpts.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/PhaseTrace.h"
#include "loganalyzers/LoadTraceFig.h"


LoadTraceFig::LoadTraceFig(): theStex(0), thePhase(0), theTrace(0) {
	theAxisY1.label("rate, #/sec");
	theAxisY2.label("bandwidth, Mbits/sec");
}

void LoadTraceFig::stats(const Stex *aStex, const PhaseInfo *aPhase) {
	theStex = aStex;
	Assert(aStex);
	thePhase = aPhase;
	theTrace = &thePhase->trace();
	Assert(theTrace);
}

void LoadTraceFig::compareWith(const Stex *stex) {
	Assert(thePhase);
	Assert(stex);
	theComparison.append(stex);
}

int LoadTraceFig::addPlotData() {
	// make sure that the most interesting line is on top
	theComparison.append(theStex);

	// create plot command
	for (int i = 0; i < theComparison.count(); ++i) {
		addPlotLine(theComparison[i]->name() + " rate");
		addPlotLine(theComparison[i]->name() + " bwidth", true);
	}

	// dump data to plot
	int pointCount = 0;
	for (int s = 0; s < theComparison.count(); ++s) {
		dumpDataLines(theComparison[s], lnRate);
		const int c = dumpDataLines(theComparison[s], lnBwidth);
		if (theStex == theComparison[s])
			pointCount = c;
	}

	return pointCount;
}

int LoadTraceFig::dumpDataLines(const LoadStex *stex, const lineType lt) {
	int pointCount = 0;
	for (int i = 0; i < theTrace->count(); ++i)
		pointCount += dumpDataLine(stex, theTrace->winPos(i), theTrace->winStats(i), lt);
	addedLineData();
	return pointCount;
}

int LoadTraceFig::dumpDataLine(const LoadStex *stex, Time stamp, const StatIntvlRec &r, const lineType lt) {
	const double val = lt == lnRate ? stex->rate(r) : stex->bwidth(r)*8/1024/1024;
	addDataPoint(stamp, val);
	return 1;
}
