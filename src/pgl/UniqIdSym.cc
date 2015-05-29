
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/UniqIdSym.h"



String UniqIdSym::TheType = "uniq_id";


UniqIdSym::UniqIdSym(const UniqId &aVal): ExpressionSym(TheType), theVal(aVal) {
}

bool UniqIdSym::isA(const String &type) const {
	return ExpressionSym::isA(type) ||
		type == TheType || type == TheType;
}

SynSym *UniqIdSym::dupe(const String &type) const {
	if (isA(type))
		return new UniqIdSym(theVal);
	return ExpressionSym::dupe(type);
}

ostream &UniqIdSym::print(ostream &os, const String &) const {
	return os << theVal;
}
