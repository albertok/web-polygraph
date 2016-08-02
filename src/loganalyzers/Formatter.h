
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_FORMATTER_H
#define POLYGRAPH__LOGANALYZERS_FORMATTER_H

#include "xstd/String.h"
#include "xstd/h/iosfwd.h"

// interface to format data for different mediums such as HTML page(s) or PDF
class Formatter {
	public:
		virtual ~Formatter() {}

		virtual void openSection(const String &id, const String &title) = 0;
		virtual void closeSection() = 0;

		virtual void openTable(const String &id, const String &title) = 0;
		virtual void openTableAnonym() = 0; // borderless, anonymous
		virtual void closeTable() = 0;

		virtual void openTableRecord() = 0;
		virtual void closeTableRecord() = 0;

		virtual void openTableCell(const String &classId) = 0;
		virtual void closeTableCell() = 0;
		virtual void addTableCell(const String &cell) = 0;

		virtual void addLink(const String &addr, const String &text) = 0;
		virtual void addText(const String &text) = 0;
		virtual void addInteger(int v, const String &unit, bool addSign = false) = 0;
		virtual void addNothing() = 0;

		// make formatted contents
		virtual void make() = 0;
};

class WebPageFormatter: public Formatter {
	public:
		WebPageFormatter(ostream *aPage);

		virtual void openSection(const String &id, const String &title);
		virtual void closeSection();

		virtual void openTable(const String &id, const String &title);
		virtual void openTableAnonym(); // borderless, anonymous
		virtual void closeTable();

		virtual void openTableRecord();
		virtual void closeTableRecord();

		virtual void openTableCell(const String &classId);
		virtual void closeTableCell();
		virtual void addTableCell(const String &cell);

		virtual void addLink(const String &addr, const String &text);
		virtual void addText(const String &text);
		virtual void addInteger(int v, const String &unit, bool addSign = false);
		virtual void addNothing();

		virtual void make();

	private:
		ostream *thePage;
};

#endif
