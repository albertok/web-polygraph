
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_JAVASCRIPTPLOTTER_H
#define POLYGRAPH__LOGANALYZERS_JAVASCRIPTPLOTTER_H

#include "loganalyzers/Plotter.h"

// javascript plotter using jqPlot
class JavascriptPlotter: public Plotter {
	public:
		JavascriptPlotter(const ReportFigure &fig);

		virtual bool start();
		virtual bool finish();

		virtual void addDataPoint(const double x, const double y);
		virtual void addedLineData();

		virtual void render(XmlTag &tag);

	protected:
		virtual void configure();

	protected:
		bool firstPointInLine;
};

#endif
