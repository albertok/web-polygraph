
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <stdlib.h>
#include "xstd/h/iomanip.h"

#include "pgl/AddrMapSym.h"
#include "pgl/DynamicNameSym.h"
#include "pgl/PglArraySym.h"
#include "runtime/AddrMapItems.h"
#include "runtime/AddrMap.h"

AddrMap *TheAddrMap = 0;

static
int cmpAddrMapItemPtr(const void *p1, const void *p2) {
	const NetAddr &a1 = (*(const AddrMapItem**)p1)->name();
	const NetAddr &a2 = (*(const AddrMapItem**)p2)->name();
	return a1.compare(a2);
}

AddrMap::AddrMap(): isSorted(true) {
}

AddrMap::~AddrMap() {
	while (theMap.count()) delete theMap.pop();
}

void AddrMap::configure(const Array<AddrMapSym*> &maps) {
	for (int i = 0; i < maps.count(); ++i) {
		AddrMapSym &ms = *maps[i];
		const ArraySym *const names = ms.namesSym();
		Addrs addrs;

		ms.addresses(addrs);

		const int nameCount = names->count();
		const int addrCount = addrs.count();

		// check that all addresses are IPs
		for (int a = 0; a < addrCount; ++a) {
			if (addrs[a]->isDomainName())
				cerr << ms.loc() << "the `address' part of an AddrMap "
					<< " must not contain domain names, got: " << *addrs[a]
					<< endl << xexit;
		}

		theMap.stretch(theMap.count() + nameCount);

		if (addrCount > 1 && nameCount == 1)
			configure1toN((const NetAddrSym &)names->item(0)->cast(NetAddrSym::TheType), addrs);
		else
		if (addrCount > 0 && nameCount >= addrCount)
			configRoundRobin(names, addrs);
		else
		if (addrCount > 0) {
			cerr << ms.loc() << "cannot map a single name to multiple " <<
				"addresses; got " << nameCount << " names and " <<
				addrCount << " addresses" << endl << xexit;
		} else {
			cerr << ms.loc() << "need at least one address and " <<
				"at least one name for address map to work; got " <<
				nameCount << " names and " << addrCount << " addresses" <<
				endl << xexit;
		}
		while (addrs.count()) delete addrs.pop();
	}

	isSorted = isSorted && !maps.count();
}

void AddrMap::config1to1(const NetAddrSym &name, const NetAddr &addr) {
	theMap.append(new Name2AddrMapItem(name, addr));
}

void AddrMap::configRoundRobin(const ArraySym* names, const Addrs &addrs) {
	for (int i = 0, a = 0; i < names->count(); ++i, ++a) {
		if (a >= addrs.count())
			a = 0;
		config1to1((const NetAddrSym &)names->item(i)->cast(NetAddrSym::TheType), *addrs[a]);
	}
}

void AddrMap::configure1toN(const NetAddrSym &name, const Addrs &addrs) {
	theMap.append(new Name2AddrsMapItem(name, addrs));
}

const AddrMapItem &AddrMap::itemAt(int nameIdx) const {
	Assert(0 <= nameIdx && nameIdx < theMap.count());
	return *theMap[nameIdx];
}

int AddrMap::nameCount() const {
	return theMap.count();
}

const NetAddr &AddrMap::nameAt(int nameIdx) const {
	return itemAt(nameIdx).name();
}

const NetAddr &AddrMap::addrAt(int nameIdx, int addrIdx) const {
	const AddrMapItem &mi = itemAt(nameIdx);
	Assert(0 <= addrIdx && addrIdx < mi.addrCount());
	return mi.addrAt(addrIdx);
}

int AddrMap::addrCountAt(int nameIdx) const {
	return itemAt(nameIdx).addrCount();
}

bool AddrMap::has(const NetAddr &name) const {
	int nameIdx = -1;
	return find(name, nameIdx);
}

bool AddrMap::find(const NetAddr &name, int &idx) const {
	if (!isSorted) {
		// sort to optimize name search later
		qsort((void*)theMap.items(), theMap.count(), sizeof(AddrMapItem*), &cmpAddrMapItemPtr);
		isSorted = true;
	}

	// binary search (should move to SortedArray or something?)
	for (int left = 0, right = theMap.count() - 1; left <= right;) {
		idx = (left + right)/2;
		const int cmp = theMap[idx]->name().compare(name);
		if (cmp == 0)
			return true;
		else 
		if (cmp < 0) // move right
			left = idx + 1;
		else
			right = idx - 1;
	}

	return false;
}

void AddrMap::add(const NetAddr &name) {
	if (name.isDomainName()) {
		cerr << here << "domain name `" << name 
			<< "' used where an IP address was expected"
			<< endl << xexit;
	}
	theMap.append(new Name2NameMapItem(name));
	isSorted = false;
}

// XXX: this needs to be optimized
bool AddrMap::findAddr(const NetAddr &addr) const {
	for (int idx = 0; idx < theMap.count(); ++idx) {
		for (AddrMapAddrIter i = addrIter(idx); i; ++i) {
			if (i.addr() == addr)
				return true;
		}
	}
	return false;
}

const NetAddr &AddrMap::selectAddr(int nameIdx) const {
	return itemAt(nameIdx).selectAddr();
}

AddrMapAddrIter AddrMap::addrIter(int nameIdx) const {
	Assert(0 <= nameIdx && nameIdx < theMap.count());
	return AddrMapAddrIter(*this, nameIdx);
}

AddrMapAddrIter AddrMap::addrIter(const NetAddr &name) const {
	int nameIdx = -1;
	Assert(find(name, nameIdx));
	return addrIter(nameIdx);
}



/* AddrMapAddrIter */

AddrMapAddrIter::AddrMapAddrIter(const AddrMap &aMap, int aName):
	theMap(aMap), theName(aName), theAddr(0) {
}

const NetAddr &AddrMapAddrIter::addr() {
	return theMap.addrAt(theName, theAddr);
}

const NetAddr &AddrMapAddrIter::name() {
	return theMap.nameAt(theName);
}

bool AddrMapAddrIter::atEnd() const {
	return theAddr >= theMap.addrCountAt(theName);
}
