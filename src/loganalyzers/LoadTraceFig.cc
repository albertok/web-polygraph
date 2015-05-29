
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

void LoadTraceFig::setCtrlOptions() {
	theLabelY1 = "rate, #/sec";
	theLabelY2 = "bandwidth, Mbits/sec";
	ReportTraceFigure::setCtrlOptions();
}

int LoadTraceFig::createCtrlFile() {
	if (ReportTraceFigure::createCtrlFile() < 0)
		return -1;

	// make sure that the most interesting line is on top
	theComparison.append(theStex);

	// create plot command
	for (int i = 0; i < theComparison.count(); ++i) {
		addPlotLine(theComparison[i]->name() + " rate", theLabelY1);
		addPlotLine(theComparison[i]->name() + " bwidth", theLabelY2);
	}
	addedAllPlotLines();

	// dump data to plot
	int pointCount = 0;
	for (int s = 0; s < theComparison.count(); ++s) {
		if (s)
			*theCtrlFile << 'e' << endl;
		dumpDataLines(theComparison[s], lnRate);
		*theCtrlFile << 'e' << endl;
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

	return pointCount;
}

int LoadTraceFig::dumpDataLine(const LoadStex *stex, Time stamp, const StatIntvlRec &r, const lineType lt) {
	const double val = lt == lnRate ? stex->rate(r) : stex->bwidth(r)*8/1024/1024;
	dumpTime(stamp);
	*theCtrlFile << ' ' << val << endl;
	return 1;
}
