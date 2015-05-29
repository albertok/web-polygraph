
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLSEARCH_H
#define POLYGRAPH__XML_XMLSEARCH_H

#include "xstd/String.h"

// searches for nodes and attributes in an XML-formatted text
// using XPath syntax (XPath support is rudimental though)
class XmlSearch {
	public:
		XmlSearch(const String &text);

		bool getInt(const String &path, int &n) const;
		bool getStr(const String &path, String &str) const;

	protected:
		const char *locate(const String &path, const char *&end) const;

	protected:
		String theText;
};

#endif
