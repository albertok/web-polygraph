
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_IMAGEPLOTTER_H
#define POLYGRAPH__LOGANALYZERS_IMAGEPLOTTER_H

#include "loganalyzers/Plotter.h"

// image plotter using gnuplot
class ImagePlotter: public Plotter {
	public:
		ImagePlotter(const ReportFigure &fig);
		virtual ~ImagePlotter();

		virtual bool start();
		virtual bool finish();

		virtual void addDataPoint(const double x, const double y);
		virtual void addedLineData();

		virtual void render(XmlTag &tag);

	protected:
		virtual void configure();

	protected:
		String theDataFname;
		ofstream theDataFile;
};

#endif
