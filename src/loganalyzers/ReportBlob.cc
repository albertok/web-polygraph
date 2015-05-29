
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xml/XmlAttr.h"
#include "loganalyzers/ReportBlob.h"

const String ReportBlob::NilTitle = "_nil";

ReportBlob::ReportBlob(const Key &aKey, const String &aTitle):
	XmlTag("report_blob"), theKey(aKey) {
	append(XmlAttr("key", aKey));
	append(XmlAttr("title", aTitle));
}

XmlNode *ReportBlob::clone() const {
	return new ReportBlob(*this);
}
