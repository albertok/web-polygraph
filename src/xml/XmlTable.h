
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLTABLE_H
#define POLYGRAPH__XML_XMLTABLE_H

#include "xml/XmlTag.h"
#include "xml/XmlNodes.h"

// split into *.h?

class XmlTableHeader;


class XmlTable: public XmlTag {
	public:
		XmlTable(): XmlTag("table") {}
};


class XmlTableRec: public XmlTag {
	public:
		XmlTableRec(): XmlTag("tr") {}
};

class XmlTableHeader: public XmlNodes {
	public:
};


class XmlTableCell: public XmlTag {
	public:
		XmlTableCell(): XmlTag("td") {}
		XmlTableCell(const String tagname): XmlTag(tagname) {}
};

class XmlTableHeading: public XmlTableCell {
	public:
		XmlTableHeading();
		XmlTableHeading(const String &text, int colspan = 1, int rowspan = 1);
};



#endif
