
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/string.h"
#include "xstd/h/sstream.h"

#include "xstd/gadgets.h"
#include "xml/XmlAttr.h"


XmlAttr XmlAttr::Int(const String &name, int val) {
	char buf[64];
	ofixedstream os(buf, sizeof(buf));
	os << val << ends;
	return XmlAttr(name, buf);
}


XmlAttr XmlAttr::Double(const String &name, double val) {
	char buf[64];
	ofixedstream os(buf, sizeof(buf));
	configureStream(os, 2);
	os << val << ends;
	return XmlAttr(name, buf);
}

XmlAttr::XmlAttr(): theNode(0) {
}

XmlAttr::XmlAttr(const XmlAttr &a): theNode(0), theName(a.theName), theValue(a.theValue) {
}

XmlAttr::XmlAttr(const String &aName, const String &aVal): 
	theNode(0), theName(aName), theValue(aVal) {
}

XmlAttr::~XmlAttr() {
	Assert(!theNode);
}

void XmlAttr::node(XmlNode *aNode) {
	Assert(!theNode || !aNode);
	theNode = aNode;
}

void XmlAttr::name(const String &aName) {
	theName = aName;
}

void XmlAttr::value(const String &aVal) {
	theValue = aVal;
}

ostream &XmlAttr::print(ostream &os, const String &pfx) const {
	os << pfx << theName << '=';
	os << '"';

	// escape double quotes
	const char *p = theValue.cstr();
	while (const char *next = strchr(p, '"')) {
		if (next > p)
			os.write(p, next - p);
		os << "&quot;";
		p = next + 1;
	}
	if (*p)
		os << p;

	os << '"';
	return os;
}
