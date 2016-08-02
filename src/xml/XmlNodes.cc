
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xml/XmlNode.h"
#include "xml/XmlAttr.h"
#include "xml/XmlAttrs.h"
#include "xml/XmlNodes.h"


XmlNodes::XmlNodes(XmlNode *aParent): theParent(aParent) {
}

XmlNodes::XmlNodes(const XmlNodes &nodes): Array<XmlNode*>(nodes.count()),
	theParent(0) {
	for (int i = 0; i < nodes.count(); ++i)
		*this << *nodes[i];
}

XmlNodes::XmlNodes(const XmlNode &node): theParent(0) {
	*this << node;
}

XmlNodes::~XmlNodes() {
	while (count()) {
		last()->parent(0);
		delete pop();
	}
}

int XmlNodes::selByTagName(const String &name, Res &res) const {
	for (int i = 0; i < count(); ++i) {
		const XmlNode *node = item(i);
		if (node->name() == name)
			res.append(node);
	}
	return res.count();
}

int XmlNodes::selByAttrName(const String &name, Res &res) const {
	for (int i = 0; i < count(); ++i) {
		const XmlNode *node = item(i);
		if (node->attrs() && node->attrs()->has(name))
			res.append(node);
	}
	return res.count();
}

int XmlNodes::selByAttrVal(const String &name, const String &value, Res &res) const {
	for (int i = 0; i < count(); ++i) {
		const XmlNode *node = item(i);
		if (node->attrs() && node->attrs()->has(name, value))
			res.append(node);
	}
	return res.count();
}

XmlNode *XmlNodes::findByAttrVal(const String &name, const String &value) {
	for (int i = 0; i < count(); ++i) {
		XmlNode *const node = item(i);
		if (node->attrs() && node->attrs()->has(name, value))
			return node;
	}
	return 0;
}

XmlNodes &XmlNodes::operator <<(const XmlAttr &a) {
	for (int i = 0; i < count(); ++i)
		item(i)->addAttr(a.clone());
	return *this;
}

XmlNodes &XmlNodes::operator <<(const XmlNode &n) {
	append(n.clone());
	last()->parent(theParent);
	return *this;
}

XmlNodes &XmlNodes::operator <<(const XmlNodes &ns) {
	stretch(count() + ns.count());
	for (int i = 0; i < ns.count(); ++i)
		*this << *ns[i];
	return *this;
}

ostream &XmlNodes::print(ostream &os, const String &pfx) const {
	for (int i = 0; i < count(); ++i)
		item(i)->print(os, pfx);
	return os;
}
