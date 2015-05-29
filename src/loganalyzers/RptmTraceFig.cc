
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>

#include "xml/XmlAttr.h"
#include "loganalyzers/Stex.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/RepOpts.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/PhaseTrace.h"
#include "loganalyzers/RptmTraceFig.h"


RptmTraceFig::RptmTraceFig(): thePhase(0), theTrace(0) {
}

void RptmTraceFig::stats(const Stex *aStex, const PhaseInfo *aPhase) {
	thePhase = aPhase;
	theTrace = &thePhase->trace();
	Assert(theTrace);
	moreStats(aStex);
}

void RptmTraceFig::moreStats(const Stex *stex) {
	if (const TmSzStat *aggr = stex->aggr(*thePhase)) {
		if (aggr->count())
			theStexes.append(stex);
	}
}

void RptmTraceFig::setCtrlOptions() {
	theLabelY1 = "msec";
	ReportTraceFigure::setCtrlOptions();
}

int RptmTraceFig::createCtrlFile() {
	if (ReportTraceFigure::createCtrlFile() < 0)
		return -1;

	// create plot command
	for (int i = 0; i < theStexes.count(); ++i)
		addPlotLine(theStexes[i]->name(), theLabelY1);
	addedAllPlotLines();

	// dump data to plot
	int pointCount = 0;
	{for (int s = 0; s < theStexes.count(); ++s) {
		if (s)
			*theCtrlFile << 'e' << endl; // note: two empty lines do not work
		pointCount += dumpDataLines(theStexes[s]);
	}}
	return pointCount;
}

int RptmTraceFig::dumpDataLines(const Stex *stex) {
	int pointCount = 0;
	for (int i = 0; i < theTrace->count(); ++i)
		pointCount += dumpDataLine(stex, theTrace->winPos(i), theTrace->winStats(i));
	return pointCount;
}

int RptmTraceFig::dumpDataLine(const Stex *stex, Time stamp, const StatIntvlRec &r) {
	const AggrStat &stat = stex->trace(r)->time();
	if (stat.count()) {
		dumpTime(stamp);
		*theCtrlFile << ' ' << stat.mean() << endl;
		return 1;
	}
	return 0;
}

