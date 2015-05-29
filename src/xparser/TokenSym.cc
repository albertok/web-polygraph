
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xparser/xparser.h"

#include "xparser/TokenSym.h"

const String TokenSym::TheType = "Token";

TokenSym::TokenSym(const String &n, int anId): 
	SynSym(TheType), theSpell(n), theId(anId) {
}

SynSym *TokenSym::dupe(const String &type) const {
	if (type != TheType)
		return SynSym::dupe(type);
	return new TokenSym(theSpell, theId);
}

ostream &TokenSym::print(ostream &os, const String &) const {
	return os << spelling();
}
