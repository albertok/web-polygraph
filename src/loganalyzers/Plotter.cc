
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "loganalyzers/Plotter.h"


Plotter::Plotter(const ReportFigure &aFig): theFig(aFig) {
}

bool Plotter::start() {
	configure();
	theCtrlFile.open(theCtrlFname.cstr());
	return theCtrlFile;
}

bool Plotter::finish() {
	theCtrlFile.close();
	return theCtrlFile;
}
