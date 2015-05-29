
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/sstream.h"

#include "xml/XmlTag.h"
#include "xml/XmlAttr.h"
#include "loganalyzers/SectionState.h"

SectionState::SectionState(): theLevels(4) {
	theLevels.append(0);
}

void SectionState::reset() {
	theLevels.reset();
	theLevels.append(0);
}

int SectionState::level() const {
	return theLevels.count();
}

// suggests section "number"
// no support for "level" attribute yet
String SectionState::begSection(const XmlTag &s, String &trueNum) {
	theLevels.last()++;

	const String numStr = curNum(s, trueNum);

	theLevels.append(0);
	return numStr;
}

void SectionState::endSection() {
	if (theLevels.count() > 1) // be robust
		theLevels.pop();
}

String SectionState::curNum(const XmlTag &s, String &trueNum) const {
	ostringstream buf;
	for (int i = 0; i < theLevels.count(); ++i) {
		if (i)
			buf << '.';
		buf << theLevels[i];
	}
	if (theLevels.count() == 1)
		buf << '.';
	buf << ends;

	trueNum = buf.str().c_str();
	streamFreeze(buf, false);

	if (const XmlAttr *a = s.attrs()->has("number"))
		return a->value();
	else
		return trueNum;
}
