
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "loganalyzers/JavascriptPlotter.h"
#include "loganalyzers/RepOpts.h"
#include "loganalyzers/ReportFigure.h"
#include "xml/XmlAttr.h"
#include "xml/XmlTable.h"


JavascriptPlotter::JavascriptPlotter(const ReportFigure &fig): Plotter(fig),
	firstPointInLine(true) {
}

void JavascriptPlotter::configure() {
	theCtrlFname = theFig.baseName() + ".js";
	// same as above but relative
	// XXX: should not be hard coded
	thePlotFname = "figures/" + theFig.key() + ".js";
}

bool JavascriptPlotter::start() {
	if (!Plotter::start())
		return false;

	return theCtrlFile <<
		"$(document).ready(function() {\n"
		"	var data = [];\n";
}

bool JavascriptPlotter::finish() {
	theCtrlFile <<
		"	var options = {\n"
		"		labelX1: '" << theFig.axisX1().label() << "',\n"
		"		labelY1: '" << theFig.axisY1().label() << "',\n"
		"		labelY2: '" << theFig.axisY2().label() << "',\n"
		"		dataStyle: '" << theFig.dataStyle() << "'\n"
		"	};\n"
		"	var lines = [];\n";

	for (ReportFigure::Lines::size_type i = 0; i < theFig.lines().size();
		++i) {
		const ReportFigure::Line &line = theFig.lines()[i];
		const char *const yaxis = line.usesY2 ? "y2axis" : "yaxis";
		theCtrlFile <<
			"	lines.push({\n"
			"		yaxis: '" << yaxis << "',\n";
		if (line.title) {
			theCtrlFile << "		label: '" <<
				line.title << "'";
		} else
			theCtrlFile << "		showLabel: false";
		if (line.isPadding) {
			theCtrlFile << ",\n"
				"		showLine: true,\n"
				"		showMarker: false";
		} else
		if (line.dataPointCount == 1) {
			theCtrlFile << ",\n"
				"		showMarker: true";
		}
		theCtrlFile << "\n"
			"	});\n";
	}

	theCtrlFile <<
		"	ReportFigures.createInteractive('" << theFig.key() << "', lines, data, options);\n"
		"});\n";

	return Plotter::finish();
}

void JavascriptPlotter::addDataPoint(const double x, const double y) {
	if (firstPointInLine) {
		firstPointInLine = false;
		theCtrlFile << "		data.push([\n";
	} else
		theCtrlFile << ",\n";

	theCtrlFile << "			[" << x << ", " << y << ']';
}

void JavascriptPlotter::addedLineData() {
	theCtrlFile << "\n"
		"		]);\n";
	firstPointInLine = true;
}

void JavascriptPlotter::render(XmlTag &tag) {
	XmlTableRec rec;
	rec << XmlAttr("type", "interactive") <<
		XmlAttr("src", thePlotFname) <<
		XmlAttr("style", "display: none");
	if (theFig.small())
		rec << XmlAttr("class", "small");

	XmlTableCell cell;
	XmlTag resizable("div");
	resizable << XmlAttr("class", "resizable");
	XmlTag target("div");
	target << XmlAttr("class", "jqplot-target");
	resizable << target;
	cell << resizable;
	rec << cell;

	tag << rec;
}
