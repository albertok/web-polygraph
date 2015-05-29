
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLNODES_H
#define POLYGRAPH__XML_XMLNODES_H

#include "xstd/Array.h"

class String;
class XmlAttr;
class XmlNode;

// search result: a set of pointers to XML nodes
class XmlSearchRes: public Array<const XmlNode*> {
	public:
		// implement these when needed:
		//int selByAttrName(const String &name, XmlSearchRes &res);
		//int selByAttrVal(const String &name, const String &value, XmlSearchRes &res);

		//ostream &print(ostream &os, const String &pfx) const;
};

// a collection of XML nodes
class XmlNodes: public Array<XmlNode*> {
	public:
		typedef XmlSearchRes Res;

	public:
		XmlNodes(XmlNode *aParent = 0);
		XmlNodes(const XmlNodes &nodes);
		XmlNodes(const XmlNode &node);
		~XmlNodes();

		int selByTagName(const String &name, Res &res) const;
		int selByAttrName(const String &name, Res &res) const;
		int selByAttrVal(const String &name, const String &value, Res &res) const;

		XmlNode *findByAttrVal(const String &name, const String &value);

		// sets the attribute for all nodes
		XmlNodes &operator <<(const XmlAttr &a);

		XmlNodes &operator <<(const XmlNode &n);
		XmlNodes &operator <<(const XmlNodes &ns);

		ostream &print(ostream &os, const String &pfx) const;

	public:
		XmlNode *theParent;
};

#endif
