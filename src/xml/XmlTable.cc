
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/sstream.h"

#include "xml/XmlAttr.h"
#include "xml/XmlText.h"
#include "xml/XmlTable.h"


XmlTableHeading::XmlTableHeading(): XmlTableCell("th") {
}

XmlTableHeading::XmlTableHeading(const String &text, int colspan, int rowspan):
	XmlTableCell("th") {

	if (colspan != 1)
		append(XmlAttr::Int("colspan", colspan));

	if (rowspan != 1)
		append(XmlAttr::Int("rowspan", rowspan));

	append(XmlText(text));
}
