
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglCodeSym.h"


String CodeSym::TheType = "Code";

CodeSym::CodeSym(const ParsSym &rawCode): ParsSym(rawCode) {
}

bool CodeSym::isA(const String &type) const {
	return ParsSym::isA(type) || type == TheType;
}

SynSym *CodeSym::dupe(const String &type) const {
	if (isA(type))
		return new CodeSym(*this);
	else
		return ParsSym::dupe(type);
}

ostream &CodeSym::print(ostream &os, const String &pfx) const {
	os << pfx << "{" << endl;
	ParsSym::print(os, pfx + '\t');
	os << pfx << "}";
	return os;
}
