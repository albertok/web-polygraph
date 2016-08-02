
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <xstd/h/os_std.h>
#include <fstream>

#include "xml/XmlAttr.h"
#include "xml/XmlText.h"
#include "xml/XmlParagraph.h"
#include "xml/XmlTable.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/RepOpts.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/ReportFigure.h"

String ReportFigure::TheBaseDir = ".";


ReportFigure::Axis::Axis(): theMinValue(-1), theMaxValue(-1),
	theLowPadding(-1), theHighPadding(-1) {
}

void ReportFigure::Axis::addDataPoint(const double v) {
	if (theMinValue == -1 || v < theMinValue)
		theMinValue = v;
	if (theMaxValue == -1 || v > theMaxValue)
		theMaxValue = v;
}

bool ReportFigure::Axis::calculatePadding() {
	if (hasData()) {
		const double epsilon = 1e-3;
		const double paddingOffset = maxValue() - minValue() < epsilon ?
			// If all values are (about) equal, add padding offset
			// of 1% of the value, but at least 1. E.g. y=0 line
			// would have padding offset of 1.
			Max(maxValue() * 0.01, 1.0) :
			// Otherwise, add padding offset of 5% of the value range.
			(maxValue() - minValue()) * 0.05;
		theHighPadding = maxValue() + paddingOffset;
		theLowPadding = minValue() - paddingOffset;
		// If minimal value is not negative, make sure minimal padding
		// value is also not negative.  I.e. avoid negative axis ranges
		// if there is no negative data values.
		if (minValue() > -epsilon && theLowPadding < 0)
			theLowPadding = 0;
		return true;
	}
	return false;
}


ReportFigure::Line::Line(const String &aTitle, const bool useY2, const bool padding):
	title(aTitle), dataPointCount(0), usesY2(useY2), isPadding(padding) {
}


ReportFigure::ReportFigure(): thePhase(0), theDataStyle("lines"), theCurLine(0),
	theImgPlotter(*this), theJsPlotter(*this), isSmall(false) {
}

void ReportFigure::configure(const String &aKey, const String &aTitle, const bool small) {
	theKey = aKey;
	theTitle = aTitle;
	theBaseName = TheBaseDir + '/' + theKey;
	isSmall = small;
}

const ReportBlob &ReportFigure::plot(BlobDb &db) {
	bool success = theImgPlotter.start() && theJsPlotter.start();
	const int pointCount = success ? addPlotData() : -1;
	if (pointCount > 0) {
		calculatePadding(theAxisX1);
		calculatePadding(theAxisY1);
		calculatePadding(theAxisY2);
		success = theImgPlotter.finish() && theJsPlotter.finish();
	} else
		success = false;

	ReportBlob blob(theKey, theTitle);

	if (success) {
		XmlTable table;
		table << XmlAttr::Int("border", 1)
			<< XmlAttr("class", "ReportFigure")
			<< XmlAttr("key", theKey);

		{
			XmlTableRec rec;
			XmlTableHeading th = XmlTableHeading();
			XmlTag header = XmlTag("div");
			header << XmlAttr("class", "header") <<
				XmlText(title());
			th << header;
			rec << th;
			table << rec;
		}

		table << theImgPlotter << theJsPlotter;

		blob << table;
		return *db.add(blob);
	} else 
	if (pointCount == 0) {
		XmlTextTag<XmlParagraph> warning;
		warning.buf() << "Figure '" << title() << "' not available: "
			<< "no datapoints to plot.";
		blob << warning;
	} else {
		XmlTextTag<XmlParagraph> error;
		error.buf() << "Figure '" << title() << "' not available: "
			<< "something went wrong while generating the graph.";
		blob << error;
	}

	return *db.add(blob);
}

void ReportFigure::addPlotLine(String title, const bool useY2, const bool padding) {
	if (useY2 && title)
		title += " (right Y axis)";
	theLines.push_back(Line(title, useY2, padding));
}

void ReportFigure::addDataPoint(const double x, const double y) {
	Must(theCurLine < theLines.size());
	++theLines[theCurLine].dataPointCount;

	theAxisX1.addDataPoint(x);
	if (theLines[theCurLine].usesY2)
		theAxisY2.addDataPoint(y);
	else
		theAxisY1.addDataPoint(y);

	theImgPlotter.addDataPoint(x, y);
	theJsPlotter.addDataPoint(x, y);
}

void ReportFigure::addedLineData() {
	++theCurLine;

	theImgPlotter.addedLineData();
	theJsPlotter.addedLineData();
}

void ReportFigure::calculatePadding(Axis &axis) {
	if (axis.calculatePadding()) {
		// add padding lines for both low and high padding points
		addPadding(axis, axis.lowPadding());
		addPadding(axis, axis.highPadding());
	}
}

// add a "fake" invisible line with a single point used for padding
void ReportFigure::addPadding(const Axis &axis, const double v) {
	const bool isXAxis = &axis == &theAxisX1;
	const Axis &otherAxis = !isXAxis ? theAxisX1 :
		theAxisY1.hasData() ? theAxisY1 : theAxisY2;
	if (Should(otherAxis.hasData())) {
		const bool useY2 = &axis == &theAxisY2 ||
			&otherAxis == &theAxisY2;
		addPlotLine("", useY2, true);
		// We need to use some otherAxis value for the padding
		// point below.  It does not matter what otherAxis
		// value is used as long as it is in [min, max] range.
		if (isXAxis)
			addDataPoint(v, otherAxis.minValue());
		else
			addDataPoint(otherAxis.minValue(), v);
		addedLineData();
	}
}
