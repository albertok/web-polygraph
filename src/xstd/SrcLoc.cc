
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iostream.h"

#include "xstd/SrcLoc.h"


SrcLoc::SrcLoc(const String &aFName, int aLineNo):
	theFName(aFName), theLineNo(aLineNo) {
}

bool SrcLoc::sameLine(const SrcLoc &t) const {
	return theFName && theFName == t.theFName && theLineNo == t.theLineNo;
}

ostream &SrcLoc::print(ostream &os) const {
	if (theFName)
		os << theFName << ':' << theLineNo << ": ";
	return os;
}
