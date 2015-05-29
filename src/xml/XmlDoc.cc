
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"
#include "xml/XmlDoc.h"

XmlDoc::XmlDoc(): theRoot(0) {
}

XmlDoc::~XmlDoc() {
	delete theRoot;
}

void XmlDoc::root(const XmlNode &aRoot) {
	Assert(!theRoot);
	theRoot = aRoot.clone();
}

ostream &XmlDoc::print(ostream &os, const String &pfx) const {
	os << pfx << "<?xml version='1.0'?>" << endl;
	if (theRoot)
		theRoot->print(os, pfx);
	return os;
}
