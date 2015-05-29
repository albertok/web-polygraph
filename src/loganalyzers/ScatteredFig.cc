
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>

#include "xstd/gadgets.h"
#include "loganalyzers/PointStex.h"
//#include "loganalyzers/ReportBlob.h"
//#include "loganalyzers/BlobDb.h"
//#include "loganalyzers/RepOpts.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/PhaseTrace.h"
#include "loganalyzers/ScatteredFig.h"


ScatteredFig::ScatteredFig(): theStex1(0), theStex2(0), thePhase(0),
	theTrace(0) {
}

void ScatteredFig::stats(const Stex *aStex1, const Stex *aStex2, const PhaseInfo *aPhase) {
	theStex1 = aStex1;
	theStex2 = aStex2;
	thePhase = aPhase;
	theTrace = &thePhase->trace();
	Assert(theStex1 && theStex2 && theTrace);
}

void ScatteredFig::setCtrlOptions() {
	theLabelX1 = theStex1->name() + ", " + theStex1->unit();
	theLabelY1 = theStex2->name() + ", " + theStex2->unit();
	theDataStyle = "points";
	ReportFigure::setCtrlOptions();
}

int ScatteredFig::createCtrlFile() {
	if (ReportFigure::createCtrlFile() < 0)
		return -1;

	addPlotLine("", theLabelY1);
	addedAllPlotLines();

	int pointCount = 0;
	for (int i = 0; i < theTrace->count(); ++i)
		pointCount += dumpDataLine(theTrace->winStats(i));

	return pointCount;
}

int ScatteredFig::dumpDataLine(const StatIntvlRec &r) {
	bool bothKnown = dumpAxis(theStex1, r);
	*theCtrlFile << ' ';
	bothKnown = dumpAxis(theStex2, r) && bothKnown;
	*theCtrlFile << endl;
	return bothKnown ? 1 : 0;
}

bool ScatteredFig::dumpAxis(const Stex *stex, const StatIntvlRec &r) {
	if (stex->valueKnown(r)) {
		*theCtrlFile << stex->value(r);
		return true;
	} else {
		*theCtrlFile << '?';
		return false;
	}
}
