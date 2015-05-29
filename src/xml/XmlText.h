
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLTEXT_H
#define POLYGRAPH__XML_XMLTEXT_H

#include "xstd/h/sstream.h"
#include "xstd/String.h"
#include "xml/XmlNode.h"

// a <tag>-less XML node
// has "text"
// never has attributes or kids
class XmlText: public XmlNode {
	public:
		XmlText();
		XmlText(const XmlText &t);
		XmlText(const String &text);

		virtual XmlNode *clone() const;

		void text(const String &text) { theBuf << text; }
		String text() const;

		ostream &buf() { return theBuf; }

		virtual const String &name() const;
		virtual XmlAttrs *attrs() { return 0; }
		virtual const XmlAttrs *attrs() const { return 0; }
		virtual const XmlNodes *kids() const { return 0; }

		virtual ostream &print(ostream &os, const String &pfx) const;
		virtual void render(XmlRenderer &r) const;

	protected:
		virtual XmlNodes *kids2() { return 0; }

	protected:
		mutable ostringstream theBuf;
};

// an XML tag initially containing nothing but a text node
template <class Parent>
class XmlTextTag: public Parent {
	public:
		XmlTextTag() { addText(); }
		XmlTextTag(const String &text) { addText(); buf() << text; }

		ostream &buf() { return theText->buf(); }

	protected:
		void addText() { (*this) << XmlText(); theText = (XmlText*)this->theKids.last(); /* XXX: cast */ }

	protected:
		XmlText *theText;
};

#endif
