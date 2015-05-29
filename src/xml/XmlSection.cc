
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xml/XmlText.h"
#include "xml/XmlAttr.h"
#include "xml/XmlSection.h"

XmlSection::XmlSection(): XmlTag("section") {
}

XmlSection::XmlSection(int level): XmlTag("section") {
	addAttr(new XmlAttr(XmlAttr::Int("level", level)));
}

XmlSection::XmlSection(int level, const String &title): XmlTag("section") {
	addAttr(new XmlAttr(XmlAttr::Int("level", level)));
	XmlTag t("title");
	t << XmlText(title);
	addChild(t.clone());
}

XmlSection::XmlSection(const String &title): XmlTag("section") {
	XmlTag t("title");
	t << XmlText(title);
	addChild(t.clone());
}
