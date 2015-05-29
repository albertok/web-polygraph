
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "xml/XmlRenderer.h"
#include "xml/XmlText.h"

XmlText::XmlText() {
	configureStream(theBuf, 2);
}

XmlText::XmlText(const String &t) {
	configureStream(theBuf, 2);
	text(t);
}

String XmlText::text() const {
	// TODO: make str()/unfreeze a method after String to std::string migration
	const string bs = theBuf.str();
	streamFreeze(theBuf, false);
	String s;
	s.append(bs.data(), bs.size());
	return s;
}

XmlNode *XmlText::clone() const {
	XmlText *txt = new XmlText;
	const string bs = theBuf.str();
	txt->buf().write(bs.data(), bs.size());
	streamFreeze(theBuf, false);
	return txt;
}

const String &XmlText::name() const {
	static const String myName("<text>"); // no tag can have a name with "<"
	return myName;
}

ostream &XmlText::print(ostream &os, const String &pfx) const {
	const string bs = theBuf.str();
	(os << pfx).write(bs.data(), bs.size());
	streamFreeze(theBuf, false);
	return os;
}

void XmlText::render(XmlRenderer &r) const {
	const string bs = theBuf.str();
	streamFreeze(theBuf, false);
	r.renderText(bs.data(), bs.size());
}
