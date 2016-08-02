
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>

#include "xml/XmlAttr.h"
#include "loganalyzers/PointStex.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/RepOpts.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/PhaseTrace.h"
#include "loganalyzers/PointTraceFig.h"


PointTraceFig::PointTraceFig(): theStex1(0), theStex2(0), thePhase(0),
	theTrace(0) {
}

void PointTraceFig::stats(const Stex *aStex1, const Stex *aStex2, const PhaseInfo *aPhase) {
	thePhase = aPhase;
	theTrace = &thePhase->trace();
	theStex1 = aStex1;
	theStex2 = (aStex2 && hasDataLines(aStex2)) ? aStex2 : 0;
	Assert(theStex1 && theTrace);

	theAxisY1.label(theStex1->unit());
	if (theStex2)
		theAxisY2.label(theStex2->unit());
}

int PointTraceFig::addPlotData() {
	addPlotLine(theStex1->name());
	if (theStex2)
		addPlotLine(theStex2->name(), true);

	int pointCount = 0;
	pointCount += dumpDataLines(theStex1);
	if (theStex2)
		pointCount += dumpDataLines(theStex2);
	return pointCount;
}

bool PointTraceFig::hasDataLines(const PointStex *stex) const {
	for (int i = 0; i < theTrace->count(); ++i) {
		const StatIntvlRec &r = theTrace->winStats(i);
		if (stex->valueKnown(r))
			return true;
	}

	return false;
}

int PointTraceFig::dumpDataLines(const PointStex *stex) {
	int pointCount = 0;
	for (int i = 0; i < theTrace->count(); ++i)
		pointCount += dumpDataLine(stex, theTrace->winPos(i), theTrace->winStats(i));
	addedLineData();
	return pointCount;
}

int PointTraceFig::dumpDataLine(const PointStex *stex, Time stamp, const StatIntvlRec &r) {
	if (stex->valueKnown(r)) {
		addDataPoint(stamp, stex->value(r));
		return 1;
	}
	return 0;
}
