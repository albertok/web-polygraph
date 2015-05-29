
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLDOC_H
#define POLYGRAPH__XML_XMLDOC_H

#include "xml/XmlNode.h"

class String;

// well-formed documents must have a single root element
// XmlDoc manages that element and provides some
// global functionality that XmlNode does not have
class XmlDoc {
	public:
		XmlDoc();
		~XmlDoc();

		XmlNode *root() { return theRoot; }
		const XmlNode *root() const { return theRoot; }
		void root(const XmlNode &aRoot);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		XmlNode *theRoot;
};

#endif
