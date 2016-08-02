
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xml/XmlAttr.h"
#include "loganalyzers/Formatter.h"

inline const String &attr(const String &value) { return value; }
inline const String &text(const String &value) { return value; }

WebPageFormatter::WebPageFormatter(ostream *aPage): thePage(aPage) {
}

void WebPageFormatter::openSection(const String &id, const String &title) {
	*thePage << "<div class=\"section\" id=\"" << attr(id) << "\">" << endl;
	*thePage << "<h2>" << text(title) << "</h2>" << endl;
}

void WebPageFormatter::closeSection() {
	*thePage << "</div>" << endl;
}

void WebPageFormatter::openTable(const String &id, const String &title) {
	*thePage << "<table id=\"" << attr(id) << "\" " <<
		"border=\"1\" cellspacing=\"0\" cellpadding=\"3\">" << endl;
	*thePage << "<tr class=\"title\"><th colspan=\"2\">" << text(title) <<
		"</th></tr>" << endl;
}

void WebPageFormatter::openTableAnonym() {
	*thePage << "<table " <<
		"border=\"0\" cellspacing=\"0\" cellpadding=\"3\">" << endl;
}

void WebPageFormatter::closeTable() {
	*thePage << "</table>" << endl;
}

void WebPageFormatter::openTableRecord() {
	*thePage << "<tr>" << endl;
}

void WebPageFormatter::closeTableRecord() {
	*thePage << "</tr>" << endl;
}

void WebPageFormatter::openTableCell(const String &classId) {
	*thePage << "<td";
	if (classId.len() > 0)
		XmlAttr("class", classId).print(*thePage, " ");
	*thePage << ">";
}

void WebPageFormatter::closeTableCell() {
	*thePage << "</td>";
}

void WebPageFormatter::addTableCell(const String &cell) {
	openTableCell("");
	addText(cell);
	closeTableCell();
}

void WebPageFormatter::addLink(const String &addr, const String &text) {
	*thePage << "<a";
	XmlAttr("href", addr).print(*thePage, " ");
	*thePage << ">";
	addText(text);
	*thePage << "</a>";
}

void WebPageFormatter::addText(const String &text) {
	*thePage << text;
}

void WebPageFormatter::addInteger(int v, const String &unit, bool addSign) {
	if (addSign && v >= 0)
		addText(v > 0 ? "+" : " ");

	*thePage << v << unit;
}

void WebPageFormatter::addNothing() {
	*thePage << " ";
}

void WebPageFormatter::make() {
	Assert(thePage);
}
