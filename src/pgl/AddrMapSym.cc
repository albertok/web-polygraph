
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/AddrMapSym.h"



String AddrMapSym::TheType = "AddrMap";

static String strAddrArr = "addr[]";
static String strAddresses = "addresses";
static String strNames = "names";
static String strZone = "zone";


AddrMapSym::AddrMapSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(StringSym::TheType, strZone, 0);
	theRec->bAdd(strAddrArr, strAddresses, 0);
	theRec->bAdd(strAddrArr, strNames, 0);
}

AddrMapSym::AddrMapSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool AddrMapSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *AddrMapSym::dupe(const String &type) const {
	if (isA(type))
		return new AddrMapSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

bool AddrMapSym::usable() const {
	const ArraySym *addrs = addressesSym();
	const ArraySym *names = namesSym();
	return addrs && names && !addrs->empty() && !names->empty();
}

String AddrMapSym::zone() const {
	return getString(strZone);
}

bool AddrMapSym::addresses(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strAddresses, addrs);
}

bool AddrMapSym::names(Array<NetAddr*> &nms) const {
	return getNetAddrs(strNames, nms);
}

const ArraySym *AddrMapSym::addressesSym() const {
	return getArraySym(strAddresses);
}

const ArraySym *AddrMapSym::namesSym() const {
	return getArraySym(strNames);
}
