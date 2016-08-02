
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/os_std.h"
#include "loganalyzers/ImagePlotter.h"
#include "loganalyzers/RepOpts.h"
#include "loganalyzers/ReportFigure.h"
#include "xml/XmlAttr.h"
#include "xml/XmlTable.h"


ImagePlotter::ImagePlotter(const ReportFigure &fig): Plotter(fig) {
}

ImagePlotter::~ImagePlotter() {
	if (theCtrlFname)
		Should(unlink(theCtrlFname.cstr()) == 0);
	if (theDataFname)
		Should(unlink(theDataFname.cstr()) == 0);
}

void ImagePlotter::configure() {
	theCtrlFname = theFig.baseName() + ".gp";
	theDataFname = theFig.baseName() + ".dat";
	thePlotFname = theFig.baseName() + ".png";
}

bool ImagePlotter::start() {
	if (!Plotter::start())
		return false;

	theDataFile.open(theDataFname.cstr());
	return theDataFile;
}

bool ImagePlotter::finish() {
	const int height = theFig.small() ? 250 : 320;
	theCtrlFile << "set term png size 640, " << height << " small" << endl;
	theCtrlFile << "set output '" << thePlotFname << "'" << endl;
	theCtrlFile << "set title ''" << endl;

	const int lineWidth = 2;
	theCtrlFile << "set style line 1 lt rgb '#0080ff' pt 1 lw " << lineWidth << endl;
	theCtrlFile << "set style line 2 lt rgb '#ff0000' pt 2 lw " << lineWidth << endl;
	theCtrlFile << "set style line 3 lt rgb '#00c000' pt 4 lw " << lineWidth << endl;
	theCtrlFile << "set style line 4 lt rgb '#c000ff' pt 5 lw " << lineWidth << endl;

	theCtrlFile << "set style line 5 lt rgb '#cccccc'" << endl;
	theCtrlFile << "set border ls 5" << endl;
	theCtrlFile << "set grid ls 5" << endl;
	theCtrlFile << "set object 1 rect from graph 0, 0 to graph 1, 1 fc rgb '#fffdf6' fs noborder behind" << endl;

	theCtrlFile << "set style data " << theFig.dataStyle() << endl;
	theCtrlFile << "set key below horizontal Left reverse" << endl;

	theCtrlFile << "set xlabel '" << theFig.axisX1().label() << "'" << endl;
	theCtrlFile << "set ylabel '" << theFig.axisY1().label() << "'" << endl;
	theCtrlFile << "set y2label '" << theFig.axisY2().label() << "'" << endl;

	theCtrlFile << "set tics textcolor rgb '#000000'" << endl;
	theCtrlFile << "set xtics out nomirror" << endl;
	theCtrlFile << "set ytics out nomirror" << endl;
	if (theFig.axisY2().label())
		theCtrlFile << "set y2tics out nomirror" << endl;

	if (theFig.axisY1().label().endsWith("%"))
		theCtrlFile << "set yrange [0:100]" << endl;
	if (theFig.axisY2().label().endsWith("%"))
		theCtrlFile << "set y2range [0:100]" << endl;

	for (ReportFigure::Lines::size_type i = 0; i < theFig.lines().size();
		++i) {
		if (!i)
			theCtrlFile << "plot '" << theDataFname << "' \\" << endl;
		else
			theCtrlFile << ", '' \\" << endl;

		theCtrlFile << "\tindex " << i;

		const ReportFigure::Line &line = theFig.lines()[i];

		if (line.usesY2)
			theCtrlFile << " axes x1y2";

		theCtrlFile << " title '";
		if (line.title)
			theCtrlFile << line.title << "  "; // poor man's padding
		theCtrlFile << "'";

		if (line.isPadding)
			theCtrlFile << " with labels"; // a hack to hide padding points
		else {
			theCtrlFile << " ls " << (i + 1);
			if (line.dataPointCount == 1)
				theCtrlFile << " with points";
		}
	}

	theCtrlFile << endl;

	if (!Plotter::finish())
		return false;

	theDataFile.close();
	if (!theDataFile)
		return false;

	const String cmd = TheRepOpts.thePlotter + " '" + theCtrlFname + '\'';
	if (::system(cmd.cstr()) != 0) {
		clog << "error: cannot plot " << theCtrlFname
			<< "; consult error messages above, if any" << endl;
		clog << "plot file was:" << endl;
		// TODO: report system(3) failure
		if (system((String("cat ") + theCtrlFname).cstr()) != 0) {}
		return false;
	}
	return true;
}

void ImagePlotter::addDataPoint(const double x, const double y) {
	theDataFile << x << ' ' << y << endl;
}

void ImagePlotter::addedLineData() {
	theDataFile << endl << endl;
}

void ImagePlotter::render(XmlTag &tag) {
	XmlTableRec rec;
	rec << XmlAttr("type", "static");

	XmlTableCell cell;
	XmlTag img("img");
	img << XmlAttr("src", thePlotFname) << XmlAttr("alt", theFig.key());
	cell << img;
	rec << cell;

	tag << rec;
}
