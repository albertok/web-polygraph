
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"
#include "loganalyzers/InfoScopeDim.h"


InfoScopeDim::InfoScopeDim(const String &aLabel): theLabel(aLabel) {
}

InfoScopeDim::InfoScopeDim(const InfoScopeDim &s) {
	copy(s);
}

InfoScopeDim::~InfoScopeDim() {
	reset();
}

String InfoScopeDim::image() const {
	String buf = theLabel;
	buf += '=';
	for (int i = 0; i < theNames.count(); ++i) {
		if (i)
			buf += ",";
		buf += *theNames[i];
	}
	return buf;
}

void InfoScopeDim::add(const String &name) {
	Assert(!has(name));
	theNames.append(new String(name));
}

void InfoScopeDim::addFront(const String &name) {
	Assert(!has(name));
	// always first; works for empty arrays too
	theNames.insert(new String(name), 0);
}

bool InfoScopeDim::has(const String &name) const {
	for (int i = 0; i < theNames.count(); ++i) {
		if (*theNames[i] == name)
			return true;
	}
	return false;
}

void InfoScopeDim::copy(const InfoScopeDim &d) {
	Assert(theLabel == d.theLabel);
	Assert(!theNames.count());
	theNames.stretch(d.count());
	for (int i = 0; i < d.count(); ++i)
		add(*d.theNames[i]);
}

void InfoScopeDim::reset() {
	while (theNames.count())
		delete theNames.pop();
}
