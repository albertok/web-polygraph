
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLATTRS_H
#define POLYGRAPH__XML_XMLATTRS_H

#include "xstd/Array.h"

class String;
class XmlAttr;

// a collection of XML nodes
class XmlAttrs: public Array<XmlAttr*> {
	public:
		typedef Array<XmlAttr*> Parent;

	public:
		const XmlAttr *has(const String &name) const;
		const XmlAttr *has(const String &name, const String &value) const;
		const String &value(const String &name) const;

		XmlAttrs &operator <<(const XmlAttr &a);
};

#endif
