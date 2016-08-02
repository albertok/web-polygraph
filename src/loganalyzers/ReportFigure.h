
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_REPORTFIGURE_H
#define POLYGRAPH__LOGANALYZERS_REPORTFIGURE_H

#include <vector>

#include "xstd/String.h"
#include "loganalyzers/ImagePlotter.h"
#include "loganalyzers/JavascriptPlotter.h"

class PhaseInfo;
class BlobDb;
class ReportBlob;
class XmlTag;

// a figure based on interval or phase stats
class ReportFigure {
	public:
		static String TheBaseDir; // where to put all figures

		class Axis {
			public:
				Axis();

				void label(const String &aLabel) { theLabel = aLabel; }
				const String &label() const { return theLabel; }
				bool hasData() const { return theMinValue != -1 && theMaxValue != -1; }
				double minValue() const { return theMinValue; }
				double maxValue() const { return theMaxValue; }
				double lowPadding() const { return theLowPadding; }
				double highPadding() const { return theHighPadding; }

				void addDataPoint(const double v);
				bool calculatePadding();

			private:
				String theLabel; // the axis label
				double theMinValue; // minimal data value, if any
				double theMaxValue; // maximum data value, if any
				double theLowPadding; // low padding value, if any
				double theHighPadding; // high padding value, if any
		};


		struct Line {
			Line(const String &aTitle, const bool useY2, const bool padding);

			String title; // the line title for the legend
			int dataPointCount; // number of data points in the line
			bool usesY2; // whether the line uses right Y axis
			bool isPadding; // whether the line is used for padding
		};

		typedef std::vector<Line> Lines;

	public:
		ReportFigure();

		const String &title() const { return theTitle; }

		virtual void configure(const String &key, const String &title, const bool small = false);

		virtual const ReportBlob &plot(BlobDb &db);

		const String &key() const { return theKey; }
		const String &baseName() const { return theBaseName; }
		const String &dataStyle() const { return theDataStyle; }
		const Axis &axisX1() const { return theAxisX1; }
		const Axis &axisY1() const { return theAxisY1; }
		const Axis &axisY2() const { return theAxisY2; }
		const Lines &lines() const { return theLines; }
		bool small() const { return isSmall; }

	protected:
		virtual int addPlotData() = 0;

		void addPlotLine(String title, const bool useY2 = false, const bool padding = false);
		void addDataPoint(const double x, const double y);
		void addedLineData();

		void calculatePadding(Axis &axis);
		void addPadding(const Axis &axis, const double v);

	protected:
		const PhaseInfo *thePhase;
		String theKey;
		String theTitle;
		String theBaseName;

		String theDataStyle;
		Axis theAxisX1;
		Axis theAxisY1;
		Axis theAxisY2;

	private:
		Lines theLines;
		Lines::size_type theCurLine;

		ImagePlotter theImgPlotter;
		JavascriptPlotter theJsPlotter;

		bool isSmall;
};

#endif
