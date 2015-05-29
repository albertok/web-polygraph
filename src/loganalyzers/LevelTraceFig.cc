
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>

#include "xml/XmlAttr.h"
#include "loganalyzers/LevelStex.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/RepOpts.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/PhaseTrace.h"
#include "loganalyzers/LevelTraceFig.h"


LevelTraceFig::LevelTraceFig(): theStex(0), thePhase(0), theTrace(0) {
}

void LevelTraceFig::stats(const LevelStex *const aStex, const PhaseInfo *const aPhase) {
	theStex = aStex;
	thePhase = aPhase;
	theTrace = &thePhase->trace();
	Assert(theTrace);
}

void LevelTraceFig::setCtrlOptions() {
	theLabelY1 = "level, #";
	ReportTraceFigure::setCtrlOptions();
}

void LevelTraceFig::compareWith(const LevelStex *const stex) {
	theComparison.append(stex);
}

int LevelTraceFig::createCtrlFile() {
	if (ReportTraceFigure::createCtrlFile() < 0)
		return -1;

	// make sure that the most interesting line is on top
	theComparison.append(theStex);

	// create plot command
	for (int i = 0; i < theComparison.count(); ++i)
		addPlotLine(theComparison[i]->name(), theLabelY1);
	addedAllPlotLines();

	int pointCount = 0;
	for (int s = 0; s < theComparison.count(); ++s) {
		if (s)
			*theCtrlFile << 'e' << endl; // note: two empty lines do not work
		pointCount += dumpDataLines(theComparison[s]);
	}

	return pointCount;
}

int LevelTraceFig::dumpDataLines(const LevelStex *stex) {
	int pointCount = 0;
	for (int i = 0; i < theTrace->count(); ++i)
		pointCount += dumpDataLine(stex, theTrace->winPos(i), theTrace->winStats(i));

	return pointCount;
}

int LevelTraceFig::dumpDataLine(const LevelStex *stex, Time stamp, const StatIntvlRec &r) {
	const LevelStat &stat = stex->level(r);
	if (stat.known()) {
		dumpTime(stamp);
		*theCtrlFile << ' ' << stat.level() << endl;
		return 1;
	}
	return 0;	
}
