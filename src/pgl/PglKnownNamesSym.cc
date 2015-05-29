
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglKnownNamesSym.h"



String KnownNamesSym::TheType = "KnownNames";



KnownNamesSym::KnownNamesSym(const String &anItemType): ArraySym(anItemType) {
}

bool KnownNamesSym::isA(const String &type) const {
	return ArraySym::isA(type) || type == TheType;
}

ArraySym *KnownNamesSym::create(const String &itemType) const {
	return new KnownNamesSym(itemType);
}
