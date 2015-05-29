
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "loganalyzers/StatTable.h"

#include "xml/XmlAttr.h"
#include "xml/XmlParagraph.h"
#include "xml/XmlText.h"

XmlTag &operator <<(XmlTag &tag, const StatTable &statTable) {
	const XmlTable &table(statTable);
	tag << table;
	const std::list<String> &unknowns(statTable.unknowns());
	if (!unknowns.empty()) {
		XmlTextTag<XmlParagraph> p;
		p.buf() << "No events observed for the following statistics:";
		for (std::list<String>::const_iterator i = unknowns.begin();
			i != unknowns.end();
			++i)
			p.buf() << (i == unknowns.begin() ? " " : ", ") << *i;
		p.buf() << '.';
		tag << p;
	}
	return tag;
}
