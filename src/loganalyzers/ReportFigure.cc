
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


ReportFigure::ReportFigure(): thePhase(0), theCtrlFile(0),
	thePlotLineCount(0) {
	theDataStyle = "lines";
}

ReportFigure::~ReportFigure() {
	delete theCtrlFile;
}

void ReportFigure::configure(const String &aKey, const String &aTitle) {
	theKey = aKey;
	theTitle = aTitle;
	theBaseName = TheBaseDir + '/' + theKey;
	thePlotFname = theBaseName + ".png";
	theCtrlFname = theBaseName + ".gp";
}

const ReportBlob &ReportFigure::plot(BlobDb &db) {
	const int pointCount = createCtrlFile();
	const bool success = pointCount > 0 && plotCtrlFile();
	destroyCtrlFile();

	ReportBlob blob(theKey, theTitle);

	if (success) {
		XmlTable table;
		table << XmlAttr::Int("border", 1);

		XmlTableRec r1;
		r1 << XmlTableHeading(title());
		table << r1;

		XmlTableRec r2;
		XmlTableCell cell;
		XmlTag img("img");
		img << XmlAttr("src", thePlotFname) << XmlAttr("alt", blob.key());
		cell << img;
		r2 << cell;
		
		table << r2;
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

int ReportFigure::createCtrlFile() {
	theCtrlFile = new ofstream(theCtrlFname.cstr());
	if (!theCtrlFile || !*theCtrlFile) {
		clog << "error: cannot create " << theCtrlFname << ": " << Error::Last() << endl;
		return false;
	}

	setCtrlOptions();
	return *theCtrlFile ? 0 : -1;
}

bool ReportFigure::plotCtrlFile() {
	const String cmd = TheRepOpts.thePlotter + ' ' + theCtrlFname +
		" > " + thePlotFname;
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

bool ReportFigure::destroyCtrlFile() {
	delete theCtrlFile;
	theCtrlFile = 0;
	return unlink(theCtrlFname.cstr()) == 0;
}

void ReportFigure::setCtrlOptions() {
	*theCtrlFile << "set term png small" << endl;
	//*theCtrlFile << "set term png color" << endl;
	*theCtrlFile << "set output '" << thePlotFname << "'" << endl;

	*theCtrlFile << "set title ''" << endl;
	*theCtrlFile << "set grid" << endl;

	*theCtrlFile << "set style line 1 lt 3" << endl;
	*theCtrlFile << "set style line 2 lt 1" << endl;
	*theCtrlFile << "set style line 3 lt 2" << endl;
	*theCtrlFile << "set style line 4 lt 4" << endl;

	*theCtrlFile << "set style data " << theDataStyle << endl;

	*theCtrlFile << "set xlabel '" << theLabelX1 << "'" << endl;
	*theCtrlFile << "set ylabel '" << theLabelY1 << "'" << endl;
	*theCtrlFile << "set y2label '" << theLabelY2 << "'" << endl;

	if (theLabelY2) {
		*theCtrlFile << "set ytics nomirror" << endl;
		*theCtrlFile << "set y2tics nomirror" << endl;
	}

	*theCtrlFile << "set size 1.0,0.5" << endl;
}

void ReportFigure::addPlotLine(const String &title, const String &unit) {
	++thePlotLineCount;
	if (thePlotLineCount == 1)
		*theCtrlFile << "plot \\" << endl;
	else
		*theCtrlFile << ", \\" << endl;

	bool useY2 = unit != theLabelY1;

	*theCtrlFile << "\t'-'";

	if (useY2)
		*theCtrlFile << " axes x1y2";

	*theCtrlFile << " title '" << title;
	if (useY2)
		*theCtrlFile << " (right Y axis)";
	*theCtrlFile << "'";

	*theCtrlFile << " with " << theDataStyle << " ls " << thePlotLineCount;
}

void ReportFigure::addedAllPlotLines() {
	*theCtrlFile << endl << endl;
}
