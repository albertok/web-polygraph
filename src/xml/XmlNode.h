
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLNODE_H
#define POLYGRAPH__XML_XMLNODE_H

#include "xstd/String.h"
#include "xstd/Array.h"

class String;
class XmlAttr;
class XmlAttrs;
class XmlNodes;
class XmlRenderer;

// an element of the XML document tree
class XmlNode {
	public:
		XmlNode();
		XmlNode(const XmlNode &n);
		virtual ~XmlNode();

		virtual XmlNode *clone() const = 0;

		const XmlNode *parent() const { return theParent; }
		void parent(XmlNode *aParent);

		virtual const String &name() const = 0;
		virtual XmlAttrs *attrs() = 0;
		virtual const XmlAttrs *attrs() const = 0;
		virtual const XmlNodes *kids() const = 0;

		void addAttr(XmlAttr *attr);
		void addChild(XmlNode *kid);

		virtual ostream &print(ostream &os, const String &pfx) const = 0;
		virtual void render(XmlRenderer &r) const = 0;

	protected:
		virtual XmlNodes *kids2() = 0;

	protected:
		XmlNode *theParent;
};

inline
ostream &operator <<(ostream &os, const XmlNode &n) {
	static String empty("");
	n.print(os, empty);
	return os;
}

#endif
