
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>

#include "xstd/gadgets.h"
#include "loganalyzers/ReportTraceFigure.h"


void ReportTraceFigure::setCtrlOptions() {
	theLabelX1 = "minutes";
	ReportFigure::setCtrlOptions();
}

void ReportTraceFigure::globalStart(Time aStart) {
	theGlobalStart = aStart;
}

// use gnuplot date axis?
void ReportTraceFigure::dumpTime(Time stamp) {
	static bool complained = false;
	complained = complained || !Should(theGlobalStart >= 0);

	*theCtrlFile << Max(0.0, (stamp - theGlobalStart).secd()/60);
}
