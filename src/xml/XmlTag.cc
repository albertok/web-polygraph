
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xml/XmlAttr.h"
#include "xml/XmlRenderer.h"
#include "xml/XmlTag.h"

XmlTag::XmlTag(const String &aName): theName(aName), theKids(this) {
}

XmlTag::XmlTag(const XmlTag &aTag): XmlNode(aTag), theName(aTag.theName), theKids(this) {
	theAttrs.stretch(aTag.theAttrs.count());
	for (int a = 0; a < aTag.theAttrs.count(); ++a)
		append(*aTag.theAttrs[a]);

	theKids.stretch(aTag.theKids.count());
	for (int k = 0; k < aTag.theKids.count(); ++k)
		append(*aTag.theKids[k]);
}

XmlTag::~XmlTag() {
	while (theAttrs.count()) {
		XmlAttr *attr = theAttrs.pop();
		attr->node(0);
		delete attr;
	}
}

XmlNode *XmlTag::clone() const {
	return new XmlTag(*this);
}

XmlAttr *XmlTag::append(const XmlAttr &a) {
	addAttr(a.clone());
	return theAttrs.last();
}

XmlNode *XmlTag::append(const XmlNode &n) {
	addChild(n.clone());
	Assert(theKids.last()->parent() == this);
	return theKids.last();
}

XmlTag &XmlTag::operator <<(const XmlAttr &a) {
	append(a);
	return *this;
}

XmlTag &XmlTag::operator <<(const XmlNode &n) {
	append(n);
	return *this;
}

XmlTag &XmlTag::operator <<(const XmlNodes &ns) {
	theKids << ns;
	return *this;
}

ostream &XmlTag::print(ostream &os, const String &pfx) const {
	printOpen(os, pfx);

	// note: we add new lines and whitespace that may not have
	// been in the original document; should we do that?
	if (theKids.count()) {
		os << '>' << endl;
		for (int i = 0; i < theKids.count(); ++i)
			theKids[i]->print(os, pfx + "\t");
		if (!theKids.last()->attrs())
			os << endl;
		os << pfx << "</" << theName << '>' << endl;
	} else {
		os << "/>" << endl;
	}

	return os;
}

void XmlTag::render(XmlRenderer &r) const {
	r.renderTag(*this);
}

void XmlTag::printOpen(ostream &os, const String &pfx) const {
	os << pfx << '<' << theName;

	for (int a = 0; a < theAttrs.count(); ++a)
		theAttrs[a]->print(os, " ");
}


