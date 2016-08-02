
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
	theDataStyle = "points";
}

void ScatteredFig::stats(const Stex *aStex1, const Stex *aStex2, const PhaseInfo *aPhase) {
	theStex1 = aStex1;
	theStex2 = aStex2;
	thePhase = aPhase;
	theTrace = &thePhase->trace();
	Assert(theStex1 && theStex2 && theTrace);

	theAxisX1.label(theStex1->name() + ", " + theStex1->unit());
	theAxisY1.label(theStex2->name() + ", " + theStex2->unit());
}

int ScatteredFig::addPlotData() {
	addPlotLine("");

	int pointCount = 0;
	for (int i = 0; i < theTrace->count(); ++i)
		pointCount += dumpDataLine(theTrace->winStats(i));
	addedLineData();
	return pointCount;
}

int ScatteredFig::dumpDataLine(const StatIntvlRec &r) {
	const bool bothKnown = theStex1->valueKnown(r) &&
		theStex2->valueKnown(r);
	if (bothKnown) {
		addDataPoint(theStex1->value(r), theStex2->value(r));
		return 1;
	}
	return 0;
}
