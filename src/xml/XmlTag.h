
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLTAG_H
#define POLYGRAPH__XML_XMLTAG_H

#include "xstd/String.h"
#include "xml/XmlNodes.h"
#include "xml/XmlAttrs.h"
#include "xml/XmlNode.h"

// an XML <tag> node 
// may have attributes and kids
// never has "text"
class XmlTag: public XmlNode {
	public:
		XmlTag(const String &aName);
		XmlTag(const XmlTag &aTag);
		virtual ~XmlTag();

		virtual XmlNode *clone() const;

		virtual const String &name() const { return theName; }

		virtual XmlAttrs *attrs() { return &theAttrs; }
		virtual const XmlAttrs *attrs() const { return &theAttrs; }
		virtual const XmlNodes *kids() const { return &theKids; }

		XmlAttr *append(const XmlAttr &a);
		XmlNode *append(const XmlNode &n);

		XmlTag &operator <<(const XmlAttr &a);
		XmlTag &operator <<(const XmlNode &n);
		XmlTag &operator <<(const XmlNodes &ns);

		virtual ostream &print(ostream &os, const String &pfx) const;
		virtual void render(XmlRenderer &r) const;

		void printOpen(ostream &os, const String &pfx) const;

	protected:
		virtual XmlNodes *kids2() { return &theKids; }

	protected:
		String theName;
		XmlAttrs theAttrs;
		XmlNodes theKids;
};

#endif
