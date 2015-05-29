
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLRENDERER_H
#define POLYGRAPH__XML_XMLRENDERER_H

class XmlDoc;
class XmlNode;
class XmlTag;
class XmlNodes;

// rendering interface
class XmlRenderer {
	public:
		virtual ~XmlRenderer() {}

		virtual void render(const XmlDoc &doc) = 0;

		void renderNode(const XmlNode &node);
		virtual void renderText(const char *buf, Size sz) = 0;
		virtual void renderTag(const XmlTag &tag) = 0;

	protected:
		void foreach(const XmlNodes *nodes);
};

#endif
