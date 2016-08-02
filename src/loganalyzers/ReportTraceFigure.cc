
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>

#include "xstd/gadgets.h"
#include "loganalyzers/ReportTraceFigure.h"


ReportTraceFigure::ReportTraceFigure() {
	theAxisX1.label("minutes");
}

void ReportTraceFigure::globalStart(Time aStart) {
	theGlobalStart = aStart;
}

// use gnuplot date axis?
void ReportTraceFigure::addDataPoint(const Time stamp, const double y) {
	static bool complained = false;
	complained = complained || !Should(theGlobalStart >= 0);
	const double x = Max(0.0, (stamp - theGlobalStart).secd()/60);
	ReportFigure::addDataPoint(x, y);
}
