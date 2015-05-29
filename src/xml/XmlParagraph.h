
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XML_XMLPARAGRAPH_H
#define POLYGRAPH__XML_XMLPARAGRAPH_H

#include "xml/XmlTag.h"

class XmlParagraph: public XmlTag {
	public:
		XmlParagraph(): XmlTag("p") {}
};

#endif
