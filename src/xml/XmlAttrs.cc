
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xml/XmlAttr.h"
#include "xml/XmlAttrs.h"

const XmlAttr *XmlAttrs::has(const String &name) const {
	for (int i = 0; i < count(); ++i) {
		if (item(i)->name() == name)
			return item(i);
	}
	return 0;
}

const XmlAttr *XmlAttrs::has(const String &name, const String &value) const {
	if (const XmlAttr *a = has(name))
		return a->value() == value ? a : 0;
	else
		return 0;
}

const String &XmlAttrs::value(const String &name) const {
	const XmlAttr *a = has(name);
	Assert(a);
	return a->value();
}

XmlAttrs &XmlAttrs::operator <<(const XmlAttr &a) {
	append(a.clone());
	return *this;
}
