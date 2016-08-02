
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_REPORTTRACEFIGURE_H
#define POLYGRAPH__LOGANALYZERS_REPORTTRACEFIGURE_H

#include "xstd/String.h"
#include "loganalyzers/ReportFigure.h"

// base class for all trace figures
class ReportTraceFigure: public ReportFigure {
	public:
		ReportTraceFigure();

		void globalStart(Time aStart);

	protected:
		void addDataPoint(const Time stamp, const double y);
		
	protected:
		Time theGlobalStart;
};

#endif
