
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ostream>

#include "xml/XmlDoc.h"
#include "xml/XmlTag.h"
#include "xml/XmlAttr.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/BlobDb.h"
#include "loganalyzers/Sample.h"
#include "loganalyzers/RepToHtmlFile.h"


PtrMap<String*> RepToHtmlFile::TheLocations;


void RepToHtmlFile::Location(BlobDb &db, const ReportBlob &blob, const String &fname) {
	TheLocations.add(blob.key(), new String(fname));
	CollectLocations(db, blob, fname);
}

void RepToHtmlFile::CollectLocations(BlobDb &db, const XmlNode &node, const String &fname) {

	if (node.attrs()) {
		if (node.name() == "report_blob" && node.attrs()->has("key")) {
			const String &key = node.attrs()->value("key");
			if (!Location(key))
				TheLocations.add(key, new String(fname + "#_" + key));
		} else
		if (node.name() == "include" && node.attrs()->has("src") &&
			node.attrs()->has("auth")) {
			const String &key = node.attrs()->value("src");
			CollectLocations(db, db.get(key), fname);
		}
	}

	if (node.kids()) {
		for (int i = 0; i < node.kids()->count(); ++i)
			CollectLocations(db, *node.kids()->item(i), fname);
	}
}

String RepToHtmlFile::Location(const String &key) {
	String *name = 0;
	if (TheLocations.find(key, name))
		return *name;
	else
		return 0;
}

RepToHtmlFile::RepToHtmlFile(BlobDb &db, ostream *aStream, const String &aLocation): 
	theDb(db), theStream(aStream), theLocation(aLocation), theQuoteLevel(0) {
}

RepToHtmlFile::~RepToHtmlFile() {
}


void RepToHtmlFile::render(const XmlDoc &doc) {
	if (doc.root()) {
		doc.root()->render(*this);
	}
}

void RepToHtmlFile::renderReportBlob(const ReportBlob &blob) {
	static const String includes = JavascriptInclude("jquery") +
		JavascriptInclude("jquery.jqplot") +
		JavascriptInclude("jqplot.canvasTextRenderer") +
		JavascriptInclude("jqplot.canvasAxisLabelRenderer") +
		JavascriptInclude("jqplot.cursor") +
		JavascriptInclude("jqplot.enhancedLegendRenderer") +
		JavascriptInclude("jquery-ui-1.8.18.custom") +
		JavascriptInclude("ReportFigures") +
		StylesheetInclude("jquery.jqplot") +
		StylesheetInclude("jquery-ui-1.8.18.custom") +
		StylesheetInclude("Print", "print") +
		StylesheetInclude("ReportFigures");
	*theStream << "<!DOCTYPE html><html><head>" << includes <<
		"</head><body>";
	renderBlob(blob);
	*theStream << "</body></html>" << endl;
}

static
void RepToHtmlFile_EscapeChar(char c, ostream &os) {
        if (c == '"')
                os << "&quot;";
        else
        if (c == '<')
                os << "&lt;";
        else
        if (c == '>')
                os << "&gt;";
        else
        if (c == '&')
                os << "&amp;";
        else
                os << c;
}

void RepToHtmlFile::renderText(const char *buf, Size sz) {
	for (int i = 0; i < sz; ++i, ++buf)
		RepToHtmlFile_EscapeChar(*buf, *theStream);
}

void RepToHtmlFile::renderTag(const XmlTag &tag) {
	if (tag.name() == "document")
		renderDocument(tag);
	else
	if (tag.name() == "section")
		renderSection(tag);
	else
	if (tag.name() == "chapter")
		renderChapter(tag);
	else
	if (tag.name() == "report_blob")
		renderBlob(tag);
	else
	if (tag.name() == "include")
		renderBlobInclude(tag);
	else
	if (tag.name() == "blob_ptr")
		renderBlobPtr(tag);
	else
	if (tag.name() == "measurement")
		renderMeasurement(tag);
	else
	if (tag.name() == "internal_error")
		*theStream << " err "; // XXX red, and add an ptr to an explanation
	else
	if (tag.name() == "codesample") {
		*theStream << "<blockquote><i><small><pre>";
		foreach(tag.kids());
		*theStream << "</pre></small></i></blockquote>";
	} else
	if (tag.name() == "title") {
		// handled inside <document> and <section>
	} else
	if (tag.name() == "description") {
		// handled inside <report_blob>
	} else
	if (tag.name() == "ul" || tag.name() == "ol") {
		renderList(tag);
	} else
	if (tag.name() == "th" || tag.name() == "td") {
		renderTableCell(tag);
	} else
	if (tag.name() == "img") {
		renderImage(tag);
	} else
	if (tag.name() == "br") {
		tag.printOpen(*theStream, "");
		*theStream << ">";
	} else {
		tag.printOpen(*theStream, "");
		*theStream << ">";
		foreach(tag.kids());
		*theStream << "</" << tag.name() << ">";
	}
}

void RepToHtmlFile::renderDocument(const XmlTag &tag) {
	XmlSearchRes res;
	if (tag.kids()->selByTagName("title", res)) {
		*theStream << "<center><h1>";
		foreach(res.last()->kids());
		*theStream << "</h1></center>";
	}
	foreach(tag.kids());
}

void RepToHtmlFile::renderChapter(const XmlTag &tag) {
	theSectionState.reset();

	if (tag.attrs()->has("name"))
		*theStream << "<a name=\"Chapter_" << tag.attrs()->value("name") << "\"></a>";

	// title
	XmlSearchRes res;
	if (Should(tag.kids()->selByTagName("title", res))) {
		*theStream << "<center><h1>";
		foreach(res.last()->kids());
		*theStream << "</h1></center>" << endl;
	}

	foreach(tag.kids());

	*theStream << endl << endl;
}

void RepToHtmlFile::renderSection(const XmlTag &tag) {
	const int sectLvl = theSectionState.level();
	String trueNum;
	const String usrNum = theSectionState.begSection(tag, trueNum);

	*theStream << "<a name=\"Sect_" << trueNum << "\"></a>";

	// title
	const int hLvl = sectLvl + 1;
	XmlSearchRes res;
	if (Should(tag.kids()->selByTagName("title", res))) {
		*theStream << "<h" << hLvl << '>' << usrNum << ' ';
		foreach(res.last()->kids());
		*theStream << "</h" << hLvl << '>' << endl;
	}

	if (sectLvl == 1)
		*theStream << "<blockquote>";

	foreach(tag.kids());

	if (sectLvl == 1)
		*theStream << "</blockquote>";

	theSectionState.endSection();
	*theStream << "<br clear='all'>" << endl;
}

void RepToHtmlFile::renderBlobInclude(const XmlTag &tag) {
	const String &key = tag.attrs()->value("src");
	const bool auth = tag.attrs()->has("auth");
	theParents.append(&tag);
	if (!auth)
		++theQuoteLevel;
	renderNode(theDb.get(key));
	if (!auth)
		--theQuoteLevel;
	theParents.pop();
}

void RepToHtmlFile::renderBlobPtr(const XmlTag &tag) {
	if (tag.attrs()->has("key")) {
	const String &key = tag.attrs()->value("key");
	if (const String loc = location(key)) {
		*theStream << "<a href=\"" << loc << "\">";
		foreach(tag.kids());
		*theStream << "</a>";
		return;
	}
	}

	*theStream << "<font color='gray'>";

	if (!tag.attrs()->has("maybe_null")) {
		const String &key = tag.attrs()->value("key");
		// XXX: we should output a link to the no-link error explanation
		if (theDb.has(key))
			cerr << "internal_error: no location for blob '" << key << "'" << endl;
		else
			cerr << "internal_error: reference to an undefined blob '" << key << "'" << endl;
	}

	foreach(tag.kids());
	*theStream << "</font>";
}

void RepToHtmlFile::renderBlob(const XmlTag &tag) {
	const String &key = tag.attrs()->value("key");
	*theStream << "<a name=\"_" << key << "\"></a>";

	const bool div = !tag.attrs()->has("dtype", "span");
	if (div)
		renderSampleStart(tag, "div", CompositeSample::TheId);

	XmlSearchRes res;
	tag.kids()->selByTagName("description", res);

	if (res.count()) {
		//*theStream << "<table border='0' align='left' cellspacing='5'><tr><td>";
		foreach(tag.kids());
		//*theStream << "</td></tr></table>";
		*theStream << "<br>" << endl;

		//*theStream << "<table border='0' align='right'><tr><td>";
		for (int i = 0; i < res.count(); ++i) {
			foreach(res[i]->kids());
			*theStream << "<br>" << endl;
		}
		//*theStream << "</td></tr></table>";
		*theStream << "<br clear='all'>" << endl;
	} else {
		foreach(tag.kids());
	}

	if (div)
		*theStream << "</div>";
}

void RepToHtmlFile::renderMeasurementVal(const XmlTag &tag, const String &val, bool renderUnit, const String &unit) {
	XmlSearchRes images;
	if (renderUnit && tag.kids()->selByTagName("image", images)) {
		foreach(images.last()->kids());
	} else {
		String typeId = unit.len() > 0 ?
			NumberSample::TheId : TextSample::TheId;
		if (const XmlAttr *a = tag.attrs()->has("typeId"))
			typeId = a->value();
		renderSampleStart(*tag.parent(), "span", typeId);
		*theStream << val;
		*theStream << "</span>";
		if (renderUnit)
			*theStream << unit;
	}
}

void RepToHtmlFile::renderMeasurement(const XmlTag &tag) {
	if (tag.attrs()->has("value")) {
		String val = tag.attrs()->value("value");
		String unit = tag.attrs()->value("unit");
		if (unit == "xact" || unit == "conn")
			val = val(0, val.chr('.') - val.cstr());
		else
		if (unit == "string")
			unit = "";

		bool renderUnit = true;
		const XmlNode *p = tag.parent();
		int parentDepth = 0;
		while (renderUnit) {
			if (!p) {
				if (parentDepth < theParents.count())
					p = theParents.last(parentDepth++);
			}
			if (!p)
				break;

			if (p->name() == "table") {
				renderUnit = p->attrs()->has("border", "0");
				break;
			}

			renderUnit = !p->attrs() || !p->attrs()->has("align", "right");
			p = p->parent();
		}

		if (!renderUnit)
			*theStream << "<small>";

		renderMeasurementVal(tag, val, renderUnit, unit);

		if (!renderUnit)
			*theStream << "</small>";
	} else {
		foreach(tag.kids());
	}
}

void RepToHtmlFile::renderList(const XmlTag &tag) {
	tag.printOpen(*theStream, "");
	*theStream << ">" << endl;

	for (int i = 0; i < tag.kids()->count(); ++i) {
		*theStream << "\t<li>";
		renderNode(*tag.kids()->item(i));
		*theStream << "</li>" << endl;
	}

	*theStream << "</" << tag.name() << ">";
}

void RepToHtmlFile::renderTableCell(const XmlTag &tag) {
	tag.printOpen(*theStream, "");
	if (tag.attrs()->has("emphasized"))
		*theStream << " bgcolor='#FFFFFF'";
	*theStream << ">";

	if (tag.kids()->count()) {
		if (tag.attrs()->has("disabled"))
			*theStream << "<font color='gray'>";
		foreach(tag.kids());
		if (tag.attrs()->has("disabled"))
			*theStream << "</font>";
	} else {
		*theStream << "&nbsp;";
	}

	*theStream << "</" << tag.name() << ">" << endl;
}

void RepToHtmlFile::renderImage(const XmlTag &tag) {
	*theStream << "<img";

	for (int a = 0; a < tag.attrs()->count(); ++a) {
		const String &name = tag.attrs()->item(a)->name();
		String value = tag.attrs()->item(a)->value();
		if (name == "src")
			value = relativeUrl(theLocation, value);
		*theStream << ' ' << name << "='" << value << "'";
	}

	*theStream << ">";
}

void RepToHtmlFile::renderSampleStart(const XmlNode &n, const String &element, const String &typeId) { 
	*theStream << "<" << element;
	if (const XmlAttrs *attrs = n.attrs()) {
		*theStream << " class=\"" << typeId << "\"";

		if (theQuoteLevel == 0) // non-authoritative includes are not IDed
			*theStream << " id=\"" << attrs->value("key") << "\"";

		const XmlAttr *title = attrs->has("title");
		if (title && title->value().len() > 0 && title->value() != ReportBlob::NilTitle)
			title->print(*theStream << " ", String());
	}
	*theStream << ">";
}

String RepToHtmlFile::relativeUrl(const String &from, const String &to) const {
	String cur = from;

	// cut ancor off
	if (const char *ancor = cur.rchr('#'))
		cur = cur(0, ancor - cur.cstr());

	// cut file name off
	if (const char *fname = cur.rchr('/'))
		cur = cur(0, fname+1 - cur.cstr());
	else
		cur = "";

	// get to the common root by replacing last dir with '..'
	String back = "";
	while (cur && !cur.casePrefixOf(to.cstr(), to.len())) {
		const char *rdir = cur.rchr('/');
		while (rdir > cur.cstr() && rdir[-1] == '/')
			--rdir;
		back += "../";
		if (cur.cstr() < rdir)
			cur = cur(0, rdir-cur.cstr());
		else
			cur = "";
	}

	const String forth = to(cur.len(), to.len());
	return back + forth;
}

String RepToHtmlFile::location(const String &key) const {
	if (const String loc = Location(key))
		return relativeUrl(theLocation, loc);
	else
		return 0;
}

String RepToHtmlFile::JavascriptInclude(const String &file) {
	static const String prefix = "<script language=\"javascript\" "
		"type=\"text/javascript\" src=\"javascripts/";
	static const String suffix = ".js\"></script>";
	return prefix + file + suffix;

}

String RepToHtmlFile::StylesheetInclude(const String &file, const String &media) {
	String s = "<link rel=\"stylesheet\" type=\"text/css\" ";
	if (media)
		s += "media=\"" + media + "\" ";
	s += "href=\"stylesheets/" + file + ".css\" />";
	return s;
}
