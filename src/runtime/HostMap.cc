
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/HostMap.h"

HostMap *TheHostMap = 0;


/* HostCfg */

HostCfg::HostCfg(const NetAddr &anAddr)
	: theAddr(anAddr), theProtocol(Agent::pUnknown),
	theContent(0), theServerRep(0), theUniverse(0),
	theHostsBasedCfg(0), isSslActive(false) {
}


/* HostMap */

HostMap::HostMap(int aCapacity): 
	theStaticIndex((aCapacity + aCapacity/3 + 7) | 1), theCount(0) {
	HostCfg *const h = 0;
	while (!theStaticIndex.full())
		theStaticIndex.push(h);
}

HostMap::~HostMap() {
	while (theStaticIndex.count())
		delete theStaticIndex.pop();
	while (theDynamicIndex.count())
		delete theDynamicIndex.pop();
}

HostCfg *HostMap::at(int idx) const {
	if (0 <= idx && idx < capacity())
		return theStaticIndex[idx];
	else
	if (capacity() <= idx && idx < 2*capacity()) {
		Assert(theDynamicIndex.capacity() == capacity());
		return theDynamicIndex[idx - capacity()];
	}
	return 0;
}

HostCfg *HostMap::at(const NetAddr &addr) {
	int idx = -1;
	Assert(findIdx(addr, idx));
	return at(idx);
}

ServerRep *HostMap::serverRepAt(int idx) {
	if (HostCfg *cfg = at(idx))
		return cfg->theServerRep;
	return 0;
}

bool HostMap::sslActive(const NetAddr &addr) {
	int idx = -1;
	if (findIdx(addr, idx))
		return at(idx)->isSslActive;
	return false;
}

ObjUniverse *HostMap::findUniverse(const NetAddr &addr) {
	int idx = -1;
	if (findIdx(addr, idx))
		return at(idx)->theUniverse;
	return 0;
}

ObjUniverse *HostMap::findUniverseAt(int idx) {
	if (const HostCfg *const h = at(idx))
		return h->theUniverse;
	return 0;
}

HostCfg *HostMap::find(const NetAddr &addr) {
	int idx = -1;
	return find(addr, idx);
}

HostCfg *HostMap::find(const NetAddr &addr, int &idx) {
	if (findIdx(addr, idx))
		return at(idx);

	return 0;
}

bool HostMap::findIdx(const NetAddr &addr, int &idx) {
	if (addr.isDynamicName() && theDynamicIndex.empty()) {
		// this is the first dynamic name, create dynamic index
		theDynamicIndex.stretch(capacity());
		HostCfg *const h = 0;
		while (!theDynamicIndex.full())
			theDynamicIndex.push(h);
	}

	if (!theDynamicIndex.empty() && addr.isDomainName())
		// for domain names, check theDynamicIndex first
		if (const char *dot = addr.addrA().chr('.')) {
			NetAddr suffixAddr(dot, addr.port());
			if (findIdxInIndex(theDynamicIndex, suffixAddr, idx)) {
				idx += capacity();
				return true;
			} else
			if (addr.isDynamicName()) {
				idx += capacity();
				return false;
			}
		}

	Assert(!addr.isDynamicName());

	return findIdxInIndex(theStaticIndex, addr, idx);
}

bool HostMap::findIdxInIndex(const Array<HostCfg*> &arr, const NetAddr &addr, int &idx) const {
	bool res = false;

	idx = hash0(addr);
	if (endSearch(arr, addr, idx, res))
		return res;

	// try hash1 followed by linear search
	idx = hash1(addr);
	for (int i = arr.capacity(); i; --i) {
		if (endSearch(arr, addr, idx, res))
			return res;
		idx++;
		idx %= arr.capacity();
	}

	Assert(false); // no empty slots left!
	return res;
}

HostCfg *HostMap::addAt(int idx, const NetAddr &addr) {
	Assert(0 <= idx && idx < 2*capacity());
	Assert(addr);
	Assert(!at(idx));
	HostCfg *const h = new HostCfg(addr);
	if (idx < capacity())
		theStaticIndex[idx] = h;
	else {
		Assert(addr.isDynamicName());
		Assert(!theDynamicIndex.empty());
		theDynamicIndex[idx - capacity()] = h;
	}
	++theCount;
	return h;
}

// returns true if there is no reason to search further (match or empty)
bool HostMap::endSearch(const Array<HostCfg*> &arr, const NetAddr &addr, int idx, bool &res) const {
	if (HostCfg *h = arr[idx]) {
		if (h->theAddr == addr)
			return res = true;
		return res = false;
	}

	// found empty slot
	res = false;
	return true;
}

int HostMap::hash0(const NetAddr &addr) const {
	return addr.hash0() % capacity();
	//return abs((int)(addr.lna() + addr.port())) % capacity();
}

int HostMap::hash1(const NetAddr &addr) const {
	return addr.hash1() % capacity();
	//return abs((int)(addr.lna() ^ addr.port() + addr.net())) % capacity();
}
