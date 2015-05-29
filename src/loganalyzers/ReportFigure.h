
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_REPORTFIGURE_H
#define POLYGRAPH__LOGANALYZERS_REPORTFIGURE_H

#include "xstd/String.h"

class String;
class PhaseInfo;
class BlobDb;
class ReportBlob;

// a figure based on interval or phase stats
class ReportFigure {
	public:
		static String TheBaseDir; // where to put all figures

	public:
		ReportFigure();
		virtual ~ReportFigure();

		const String &title() const { return theTitle; }

		virtual void configure(const String &key, const String &title);

		virtual const ReportBlob &plot(BlobDb &db);

	public:
		// set before calling setCtrlOptions
		String theDataStyle;
		String theLabelX1;
		String theLabelY1;
		String theLabelY2;

	protected:
		virtual int createCtrlFile() = 0;
		virtual void setCtrlOptions() = 0;
		bool plotCtrlFile();
		bool destroyCtrlFile();

		void addPlotLine(const String &title, const String &unit);
		void addedAllPlotLines();
		
	protected:
		const PhaseInfo *thePhase;
		String theKey;
		String theTitle;
		String theBaseName;
		String thePlotFname;
		String theCtrlFname;

		ostream *theCtrlFile;
		int thePlotLineCount;
};

#endif
