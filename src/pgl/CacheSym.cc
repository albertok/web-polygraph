
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/CacheSym.h"



String CacheSym::TheType = "Cache";

static const String strCapacity = "capacity";
static const String strIcp_port = "icp_port";

CacheSym::CacheSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(SizeSym::TheType, strCapacity, 0);
	theRec->bAdd(IntSym::TheType, strIcp_port, 0);
}

CacheSym::CacheSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool CacheSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *CacheSym::dupe(const String &type) const {
	if (isA(type))
		return new CacheSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

bool CacheSym::capacity(BigSize &cap) const {
	return getSize(strCapacity, cap);
}

bool CacheSym::icpPort(int &port) const {
	return getInt(strIcp_port, port);
}
