
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xml/XmlText.h"
#include "xml/XmlTag.h"
#include "xml/XmlRenderer.h"

void XmlRenderer::renderNode(const XmlNode &node) {
	if (node.kids()) {
		renderTag((const XmlTag&)node);
	} else {
		const String text = ((const XmlText &)node).text();
		renderText(text.cstr(), text.len());
	}
}

void XmlRenderer::renderTag(const XmlTag &tag) {
	foreach(tag.kids());
}

void XmlRenderer::foreach(const XmlNodes *nodes) {
	if (nodes) {
		for (int i = 0; i < nodes->count(); ++i)
			nodes->item(i)->render(*this);
	}
}
