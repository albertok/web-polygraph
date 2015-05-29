
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"
#include "xml/XmlAttr.h"
#include "xml/XmlAttrs.h"
#include "xml/XmlNodes.h"
#include "xml/XmlNode.h"


XmlNode::XmlNode(): theParent(0) {
}

XmlNode::XmlNode(const XmlNode &): theParent(0) {
}

XmlNode::~XmlNode() {
	Assert(!theParent);
}

void XmlNode::parent(XmlNode *aParent) {
	Assert(!theParent || !aParent);
	theParent = aParent;
}

void XmlNode::addAttr(XmlAttr *attr) {
	XmlAttrs *as = attrs();
	Assert(as);
	Assert(!as->has(attr->name()));

	attr->node(this);
	as->append(attr);
}

void XmlNode::addChild(XmlNode *child) {
	XmlNodes *ns = kids2();
	Assert(ns);
	child->parent(this);
	ns->append(child);
}

