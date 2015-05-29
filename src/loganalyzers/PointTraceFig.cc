
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
}

void PointTraceFig::setCtrlOptions() {
	theLabelY1 = theStex1->unit();
	if (theStex2)
		theLabelY2 = theStex2->unit();
	ReportTraceFigure::setCtrlOptions();
}

int PointTraceFig::createCtrlFile() {
	if (ReportTraceFigure::createCtrlFile() < 0)
		return -1;

	addPlotLine(theStex1->name(), theStex1->unit());
	if (theStex2)
		addPlotLine(theStex2->name(), theStex2->unit());
	addedAllPlotLines();

	int pointCount = 0;
	pointCount += dumpDataLines(theStex1);
	*theCtrlFile << 'e' << endl;
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

	return pointCount;
}

int PointTraceFig::dumpDataLine(const PointStex *stex, Time stamp, const StatIntvlRec &r) {
	dumpTime(stamp);
	if (stex->valueKnown(r)) {
		*theCtrlFile << ' ' << stex->value(r) << endl;
		return 1;
	} else {
		*theCtrlFile << ' ' << '?' << endl;
		return 0;
	}
}
