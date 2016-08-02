
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "base/ObjId.h"
#include "client/PrivCache.h"


PrivCache::PrivCache(int aCapacity): theEntryCount(0), theHitCount(0), theMissCount(0) {
	if (aCapacity > 0)
		capacity(aCapacity);
}

void PrivCache::clear() {
	theHistory.reset();
	theHash.clear();
	theEntryCount = 0;
	// theHitCount = theMissCount = 0; // need to handle stats differently
}

bool PrivCache::loadOid(const ObjId &oid) {
	const int idx = index(hash(oid));
	const Entry &e = theHash[idx];
	if (e.key == oid.hash()) {
		if (oid.basic()) {
			++theHitCount;
			return true;
		}
		--theEntryCount;
		delAt(idx); // purge stale or because request demands it
	}

	theMissCount++;
	return false;
}

bool PrivCache::storeOid(const ObjId &oid) {
// XXX: implement
	addOid(oid);
	return true;
}

bool PrivCache::purgeOid(const ObjId &oid) {
	const int idx = index(hash(oid));
	const Entry &e = theHash[idx];
	if (e.key == oid.hash()) {
		--theEntryCount;
		delAt(idx);
		return true;
	}
	return false;
}

void PrivCache::capacity(int aCap) {
	Assert(aCap >= 0);
	Assert(!theHash.count());

	theHistory.resize(aCap);

	// would be nice to find the closest prime
	aCap = 3*aCap | 1;
	Assert(aCap <= 0xFFFF); // we use short int as an index
	theHash.resize(aCap);
}

int PrivCache::hash(const ObjId &oid) const {
	return oid.hash();
}

int PrivCache::index(int key) const {
	return key % theHash.capacity();
}

bool PrivCache::hasOid(const ObjId &oid) const {
	return theHash[index(hash(oid))].key == oid.hash();
}

void PrivCache::addOid(const ObjId &oid) {
	Assert(oid.name() > 0 || oid.foreignUrl());

	while (theEntryCount > theHistory.capacity())
		purgeOne();

	// the entry count may not reflect history size: collisions are not purged
	if (theHistory.full()) 
		purgeOne();

	const int idx = index(hash(oid));
	Entry &e = theHash[idx];
	if (e) // collision simply overwrites an older entry w/o history update
		--theEntryCount;
	e.key = oid.hash();

	theHistory.enqueue((HistoryItem)e.key);
	++theEntryCount;
}

void PrivCache::delAt(int idx) {
	theHash[idx].key = -1;
}

void PrivCache::purgeOne() {
	const int key = theHistory.dequeue();
	const int idx = index(key);
	if (theHash[idx].key == key) // else we decremented the count on collision
		--theEntryCount;
	delAt(idx);
}
