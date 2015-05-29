
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/string.h"

#include "xstd/gadgets.h"
#include "xml/XmlSearch.h"


XmlSearch::XmlSearch(const String &aText): theText(aText) {
}

bool XmlSearch::getInt(const String &path, int &n) const {
	const char *eoa = 0;
	if (const char *pos = locate(path, eoa)) {
		const char *eon = 0;
		return isInt(pos, n, &eon) && eon == eoa;
	}
	return false;
}

bool XmlSearch::getStr(const String &path, String &str) const {
	const char *eos = 0;
	if (const char *pos = locate(path, eos)) {
		str = String();
		str.append(pos, eos - pos);
		return true;
	}
	return false;
}

// XXX: this implementation supports the most simple case of
// unique attribute name only
const char *XmlSearch::locate(const String &path, const char *&valEnd) const {
	const char *attrName = path.chr('@');
	Assert(attrName);
	attrName++;

	const char *attrStart = theText.str(attrName);
	if (!attrStart)
		return 0;

	const char *attrEnd = strchr(attrStart, '=');
	if (!attrEnd)
		return 0;

	const char qw = attrEnd[1];
	if (qw != '\'' && qw != '\"')
		return 0;

	const char *valStart = attrEnd+2;
	valEnd = strchr(valStart, qw);
	if (!valEnd)
		return 0;

	return valStart;
}


#ifdef FUTURE_CODE

Scope XmlSearch::locate(const XPath &path, Scope scope) const {
	for (int i = 0; scope && i < path.count(); ++i) {
		scope = locate(path[i], scope);
	}
	return scope;
}

Scope XmlSearch::locate(const XPathItem &pi, Scope scope) const {
	if (pi.theType == XPathItem::pitNode) {
		Scope altScope;
		while (Scope newScope = findNode(scope, altScope)) {
			if (mynode)
				return newScope;
			else
				scope = altScope;
		}
		return scope;
	}

	if (pi.theType == XPathItem::pitAttr) {
		Scope altScope;
		while (Scope newScope = findAttr(scope, altScope)) {
			if (myattr)
				return newScope;
			else
				scope = altScope;
		}
		return scope;
	}

	Assert(false);
	return scope;
}

#endif
