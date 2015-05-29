
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
		void globalStart(Time aStart);

		void setCtrlOptions();

	protected:
		void dumpTime(Time stamp);
		
	protected:
		Time theGlobalStart;
};

#endif
