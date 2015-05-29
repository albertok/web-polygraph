
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLATTR_H
#define POLYGRAPH__XML_XMLATTR_H

#include "xstd/String.h"

class XmlNode;

// XML tag's name="value" pair
class XmlAttr {
	public:
		static XmlAttr Int(const String &name, int val);
		static XmlAttr Double(const String &name, double val);

	public:
		XmlAttr();
		XmlAttr(const XmlAttr &attr);
		XmlAttr(const String &aName, const String &aVal);
		~XmlAttr();

		XmlAttr *clone() const { return new XmlAttr(theName, theValue); }

		const String &name() const { return theName; }
		const String &value() const { return theValue; }

		void node(XmlNode *aNode);
		void name(const String &aName);
		void value(const String &aVal);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		XmlNode *theNode;
		String theName;
		String theValue;
};

#endif
