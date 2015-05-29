
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ObjId.h"
#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "runtime/polyErrors.h"
#include "pgl/CacheSym.h"
//#include "client/CltXact.h"
#include "cache/DistrPoint.h"
#include "cache/CacheEntry.h"
#include "cache/CacheEntryHash.h"
#include "cache/CachePolicy.h"
#include "cache/Cache.h"




ObjFarm<CacheEntry> Cache::TheEntries;


/* CacheUser */

CacheUser::CacheUser(): theDistrPoint(0) {
}

CacheUser::~CacheUser() {
	Assert(!theDistrPoint);
}

void CacheUser::reset() {
	Assert(!theDistrPoint);
}


/* Cache */

Cache::Cache(): theIndex(0), thePolicy(0), theClient(0),
	theCapacity(0), theSize(0) {
	thePolicy = new CachePolicy(this);
}

Cache::~Cache() {
	delete theIndex; /* XXX: should delete all entries first */
	delete thePolicy;
}

void Cache::configure(const CacheSym *cfg) {
	// re-configure might require extra work; prevent for now
	Assert(theCapacity == 0 && !theIndex);

	cfg->capacity(theCapacity);
	const BigSize avgObjSize(10240); /* XXX */
	const int cap = (int)(theCapacity/avgObjSize);

	Comment(5) << "pre-allocating cache structures for at least " << cap << " objects" << endc;

	// XXX: this wastes the farm array when most of the entries
	// get moved to the index
	TheEntries.limit(cap);
	TheEntries.populate(cap);

	theIndex = new CacheEntryHash(cap);
}

void Cache::client(Client *aClient) {
	Assert(!theClient);
	theClient = aClient;
}

CacheEntry *Cache::cached(const ObjId &id) const {
	CacheEntryHash::Loc loc;
	return theIndex->find(id, loc) ? *loc : 0;
}

bool Cache::cacheEntry(CacheEntry *e) {
	Assert(e);
	CacheEntryHash::Loc loc;

	// check if already cached
	if (theIndex->find(e->id(), loc))
		return false;

	if (!thePolicy->canAdmit(e))
		return false;

	const BigSize sz(e->objSize());
	Assert(sz >= 0);
	if (theCapacity < sz)
		return false;

	// purge old entries to free space if needed
	while (space() < sz) {
		CacheEntry *v = thePolicy->nextVictim();
		Assert(purgeEntry(v));
		if (v->stray())
			noteStrayEntry(v);
	}

	theIndex->add(e);
	theSize += sz;
	thePolicy->noteAdmitted(e);
	return true;
}

bool Cache::purgeEntry(CacheEntry *e) {
	Assert(e);

	// remove from the cache index
	CacheEntryHash::Loc loc;
	if (!theIndex->find(e->id(), loc))
		return false;
	theIndex->delAt(loc);

	// update size
	const BigSize sz(e->objSize());
	Assert(sz >= 0);
	theSize -= sz;

	// notify others
	thePolicy->notePurged(e);

	Assert(space() > 0);
	return true;
}

DistrPoint *Cache::addReader(const ObjId &id, CacheReader *r) {
	Assert(r);
	if (DistrPoint *dp = getDistrPoint(id, false)) {
		if (dp->reader()) { // XXX: hack to prevent multiple readers, remove!
			//ReportError(errUnchbHit);
			return 0;
		}

		dp->addReader(r);
		Assert(!dp->writer()); // must be a hit; misses handled below
		Assert(dp->readySize() >= 0); // hits must have known sizes
		return dp;
	} else {
		// miss case
		//ReportError(errSiblingViolation); // XXX: make it an option
		dp = getDistrPoint(id, true); // XXX: called twice if miss
		dp->addReader(r);
		dp->mustProvideWriter();
		return dp;
	}
}

DistrPoint *Cache::addWriter(const ObjId &id, CacheWriter *w) {
	Assert(w);
	DistrPoint *dp = getDistrPoint(id, true);
	Assert(dp);
	dp->addWriter(w);
	return dp;
}

void Cache::noteStrayEntry(CacheEntry *e) {
	Assert(e);
	TheEntries.put(e);
}

DistrPoint *Cache::getDistrPoint(const ObjId &id, bool canCreate) {
	CacheEntryHash::Loc loc;
	CacheEntry *e = 0;
	if (theIndex->find(id, loc)) {
		e = *loc;
	} else 
	if (canCreate) {
		e = TheEntries.get();
		e->id(id);
		// do not cache until we know it is cachable
	} else {
		return 0;
	}

	Assert(e);
	DistrPoint *dp = e->distrPoint();
	if (!dp)
		dp = e->startDistributing(this);

	Assert(dp);
	return dp;
}
