
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/RangeSym.h"



const String RangeSym::TheType = "Range";

RangeSym::RangeSym(): RecSym(TheType, new PglRec) {
}

RangeSym::RangeSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool RangeSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *RangeSym::dupe(const String &type) const {
	if (isA(type))
		return new RangeSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}
