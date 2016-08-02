
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

//#include "base/StringArray.h"
#include "pgl/PglStringArrayBlockSym.h"

const String StringArrayBlockSym::TheType = "StringArrayBlock";


StringArrayBlockSym::StringArrayBlockSym(const String &aType): ContainerSym(aType) {
}

bool StringArrayBlockSym::isA(const String &type) const {
	return type == TheType || ContainerSym::isA(type);
}

void StringArrayBlockSym::forEach(Visitor &v, RndGen *const rng) const {
	// leaf; the visitor may use strings() to continue
	v.visit(*this);
}
