
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglStringSym.h"



String StringSym::TheType = "string";


StringSym::StringSym(const String &aVal): ExpressionSym(TheType), theVal(aVal) {
}

bool StringSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *StringSym::dupe(const String &type) const {
	if (isA(type))
		return new StringSym(theVal);
	return ExpressionSym::dupe(type);
}

ostream &StringSym::print(ostream &os, const String &) const {
	return os << '"' << theVal << '"';
}
