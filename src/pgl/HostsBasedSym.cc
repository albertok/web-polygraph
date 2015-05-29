
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/HostsBasedSym.h"


String HostsBasedSym::TheType = "HostsBased";

static String strAddrArr = "addr[]";
static String strAddresses = "addresses";
static String strHosts = "hosts";


HostsBasedSym::HostsBasedSym(const String &aType): RecSym(aType, new PglRec) {
	// originally, strAddresses were strHosts, and we did not want to
	// change class name when the field name has changed 
	// to avoid renaming a bunhc of files
	theRec->bAdd(strAddrArr, strAddresses, 0);
}

HostsBasedSym::HostsBasedSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool HostsBasedSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *HostsBasedSym::dupe(const String &type) const {
	if (isA(type))
		return new HostsBasedSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

int HostsBasedSym::hostCount() const {
	if (ArraySym *a = getArraySym(strAddresses))
		return a->count();
	else
		return 0;
}

NetAddrSym *HostsBasedSym::host(int idx) const {
	if (ArraySym *a = getArraySym(strAddresses)) {
		Assert(idx < a->count());
		return &(NetAddrSym&)(*a)[idx]->cast(NetAddrSym::TheType);
	}
	return 0;
}

const ArraySym *HostsBasedSym::addresses() const {
	return getArraySym(strAddresses);
}

bool HostsBasedSym::addresses(ArraySym &addrs) const {
	if (const ArraySym *a = addresses()) {
		addrs.append(*a);
		return true;
	}
	return false;
}
