
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/PglNetAddrRangeSym.h"



const String NetAddrRangeSym::TheType = "NetAddrRange";

static String strAddrArr = "addr[]";


NetAddrRangeSym::NetAddrRangeSym(): ContainerSym(TheType),
	theRange(0), theAddr(0) {
}

NetAddrRangeSym::~NetAddrRangeSym() {
	delete theRange;
	delete theAddr;
}

bool NetAddrRangeSym::isA(const String &type) const {
	return type == TheType || ContainerSym::isA(type);
}

SynSym *NetAddrRangeSym::dupe(const String &type) const {
	if (isA(type)) {
		NetAddrRangeSym *clone = new NetAddrRangeSym;
		if (theRange)
			clone->range(new PglNetAddrRange(*theRange));
		return clone;
	} else
	if (type == strAddrArr) {
		ArraySym *arr = new ArraySym(NetAddrSym::TheType);
		arr->add(*this);
		return arr;
	} else {
		return 0;
	}
}

int NetAddrRangeSym::count() const {
	Assert(theRange);
	return theRange->count();
}

bool NetAddrRangeSym::probsSet() const {
	return false; // we do not support probabilities for address ranges yet
}

const SynSym *NetAddrRangeSym::itemProb(int idx, double &prob) const {
	Assert(theRange);

	delete theAddr;
	theAddr = new NetAddrSym;
	theRange->addrAt(idx, *theAddr);

	const int cnt = theRange->count();
	Assert(cnt > 0);
	prob = 1.0/cnt;
	return theAddr;
}

void NetAddrRangeSym::forEach(Visitor &v, RndGen *const rng) const {
	ContainerSym::forEach(v, rng);
}

ostream &NetAddrRangeSym::print(ostream &os, const String &) const {
	Assert(theRange);
	os << '\'';
	theRange->print(os);
	os << '\'';
	return os;
}
