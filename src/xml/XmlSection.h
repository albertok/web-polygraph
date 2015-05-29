
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLSECTION_H
#define POLYGRAPH__XML_XMLSECTION_H

#include "xml/XmlTag.h"

class XmlSection: public XmlTag {
	public:
		XmlSection();
		XmlSection(int level);
		XmlSection(const String &title);
		XmlSection(int level, const String &title);

	protected:
		static String TagName(int level);
};



#endif
