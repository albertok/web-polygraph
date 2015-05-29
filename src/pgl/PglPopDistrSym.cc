
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/Assert.h"
#include "xstd/String.h"
#include "pgl/PopDistr.h"
#include "pgl/PglPopDistrSym.h"



const String PopDistrSym::TheType = "PopDistr";



// XXX: we are leaking pop distributions here as they are never deleted!

PopDistrSym::PopDistrSym(const String &aType, PopDistr *aVal): 
	ExpressionSym(aType), theVal(aVal) {
}

PopDistrSym::~PopDistrSym() {
}

bool PopDistrSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *PopDistrSym::dupe(const String &type) const {
	if (isA(type)) {
		PopDistrSym *d = new PopDistrSym(this->type(), theVal);
		return d;
	}
	return ExpressionSym::dupe(type);
}

ostream &PopDistrSym::print(ostream &os, const String &) const {
	if (theVal)
		theVal->print(os);
	else
		os << "<none>";
	return os;
}
