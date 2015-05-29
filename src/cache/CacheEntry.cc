
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "cache/DistrPoint.h"
#include "cache/CacheEntry.h"
#include "cache/Cache.h"

CacheEntry::CacheEntry(): nextInIdx(0), theDistrPoint(0) {
	reset();
}

CacheEntry::CacheEntry(const ObjId &anId): nextInIdx(0), theDistrPoint(0) {
	reset();
	theId = anId;
}

CacheEntry::~CacheEntry() {
	// paranoid asserts to check that we cleaned the entry before
	// destroying it
	Assert(cacheEntries.isolated());
	Assert(!theDistrPoint);
	Assert(!nextInIdx);
}

void CacheEntry::reset() {
	Assert(!theDistrPoint);
	cacheEntries.isolate();
	theDistrPoint = 0;
	theId.reset();
	theObjSize = -1;
	nextInIdx = 0;
}

DistrPoint *CacheEntry::startDistributing(Cache *cache) {
	Assert(cache);

	theDistrPoint = new DistrPoint(cache, this); // Farm these?
	if (theObjSize >= 0)
		theDistrPoint->noteDataReady(theObjSize);
	return theDistrPoint;
}

void CacheEntry::stopDistributing(DistrPoint *dp) {

	Assert(dp == theDistrPoint);
	Cache *cache = theDistrPoint->cache();
	Assert(cache);

	delete theDistrPoint;
	theDistrPoint = 0;

	if (stray())
		cache->noteStrayEntry(this); // will destroy
}
