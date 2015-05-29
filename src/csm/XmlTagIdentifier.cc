
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "csm/XmlTagIdentifier.h"
#include "csm/XmlTagParser.h"

XmlTagIdentifier::XmlTagIdentifier() {
}

XmlTagIdentifier::~XmlTagIdentifier() {
	while (theAttrIndeces.count())
		delete theAttrIndeces.pop();
}

void XmlTagIdentifier::configure(Array<String*> &tags) {
	for (int i = 0; i < tags.count(); ++i)
		addTag(*tags[i]);
	theTagIdx.optimize();
}

bool XmlTagIdentifier::findTag(const char *nameBeg, int nameLen, int &id) const {
	if (const int i = theTagIdx.lookup(nameBeg, nameLen)) {
		id = i;
		return true;
	}
	return false;
}

bool XmlTagIdentifier::findAttr(int tagId, const char *nameBeg, int nameLen) const {
	const int tagIdx = tagId - 1;
	Assert(0 <= tagIdx && tagIdx < theAttrIndeces.count());
	const PrefixIdentifier *identifier = theAttrIndeces[tagIdx];
	Assert(identifier);
	return identifier->lookup(nameBeg, nameLen) > 0;
}

void XmlTagIdentifier::addTag(const String &str) {
	static XmlTagParser parser;
	const bool goodFormat = str.len() > 2 && 
		str[0] == '<' && str.last() == '>' &&
		parser.parse(str.data()+1, str.data() + str.len()-1);
	if (goodFormat) { 
		const XmlTagParser::Token &tagname = parser.tagname();
		const int tagId  = theTagIdx.add(
			String(tagname.nameBeg, tagname.nameLen));

		PrefixIdentifier *identifier = new PrefixIdentifier;
		for (int i = 0; i < parser.attrCount(); ++i) {
			const XmlTagParser::Token &attr = parser.attr(i);
			identifier->add(String(attr.nameBeg, attr.nameLen));
		}
		identifier->optimize();
		theAttrIndeces.put(identifier, tagId-1);

		if (!parser.attrCount()) {
			cerr << here << "no <tag> attributes near '"
			<< str << "', expected <name attr=value ...>" << endl;
		}
	} else {
		if (parser.error())
			cerr << parser.error();
		else
			cerr << "invalid markup tag format";
		cerr << " near '" << str << "', expected <name attr ...>" << endl;
		exit(-2);
	}
}

XmlTagIdentifier::XmlTagIdentifier(const XmlTagIdentifier&) {
	Assert(false); // disabled
}
