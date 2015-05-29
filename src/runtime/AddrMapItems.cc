
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Rnd.h"
#include "pgl/AddrMapSym.h"
#include "pgl/DynamicNameSym.h"
#include "runtime/AddrMapItems.h"


/* AddrMapItem */

AddrMapItem::AddrMapItem(const NetAddr &aName): theName(aName), theNewProb(-1) {
	Assert(theName);
}

AddrMapItem::AddrMapItem(const NetAddrSym &aName): theName(aName.val()) {
	Assert(theName);
	if (aName.isA(DynamicNameSym::TheType)) {
		Assert(theName.isDynamicName());
		((const DynamicNameSym&)aName).newProb(theNewProb);
	} else {
		Assert(!theName.isDynamicName());
		theNewProb = -1;
	}
}


/* Name2NameMapItem */

Name2NameMapItem::Name2NameMapItem(const NetAddr &aName): AddrMapItem(aName) {
	Assert(!theName.isDomainName());
}


/* Name2AddrMapItem */

Name2AddrMapItem::Name2AddrMapItem(const NetAddrSym &aName, const NetAddr &anAddr):
	AddrMapItem(aName), theAddr(anAddr) {
	Assert(theAddr);
	Assert(!theAddr.isDomainName());
}


/* Name2AddrsMapItem */

Name2AddrsMapItem::Name2AddrsMapItem(const NetAddrSym &aName, const Array<NetAddr*> &anAddrs):
	AddrMapItem(aName) {
	theAddrs.stretch(anAddrs.count());
	for (int i = 0; i < anAddrs.count(); ++i) {
		const NetAddr &addr = *anAddrs[i];
		Assert(!addr.isDomainName());
		theAddrs.append(new NetAddr(addr));
	}
}

Name2AddrsMapItem::~Name2AddrsMapItem() {
	while (theAddrs.count()) delete theAddrs.pop();
}

const NetAddr &Name2AddrsMapItem::selectAddr() const {
	static RndGen rng;
	const int idx = rng(0, theAddrs.count());
	return addrAt(idx);
}
