
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_PLOTTER_H
#define POLYGRAPH__LOGANALYZERS_PLOTTER_H

#include <fstream>

#include "xstd/String.h"

class ReportFigure;
class XmlTag;

// general plotter interface
class Plotter {
	public:
		Plotter(const ReportFigure &aFig);
		virtual ~Plotter() {}

		virtual bool start();
		virtual bool finish();

		virtual void addDataPoint(const double x, const double y) = 0;
		virtual void addedLineData() = 0;

		virtual void render(XmlTag &tag) = 0;

	protected:
		virtual void configure() = 0;

	protected:
		const ReportFigure &theFig;
		String theCtrlFname;

		ofstream theCtrlFile;
		String thePlotFname;
};

inline
XmlTag &operator <<(XmlTag &tag, Plotter &plotter) {
	plotter.render(tag);
	return tag;
}

#endif
