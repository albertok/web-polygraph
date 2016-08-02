
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/DnsCacheSym.h"



String DnsCacheSym::TheType = "DnsCache";

static const String strCapacity = "capacity";
static const String strSharing_group = "sharing_group";

DnsCacheSym::DnsCacheSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(IntSym::TheType, strCapacity, 0);
	theRec->bAdd(StringSym::TheType, strSharing_group, 0);
}

DnsCacheSym::DnsCacheSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool DnsCacheSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *DnsCacheSym::dupe(const String &type) const {
	if (isA(type))
		return new DnsCacheSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

bool DnsCacheSym::capacity(int &cap) const {
	return getInt(strCapacity, cap);
}

String DnsCacheSym::sharingGroup() const {
	return getString(strSharing_group);
}
