
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CACHE_CACHEENTRYHASH_H
#define POLYGRAPH__CACHE_CACHEENTRYHASH_H

#include "base/ObjId.h"

class CacheEntry;

// a hash for cache entries
// we would use a "generic" hash, but its implementation via
// templates is probably way to ugly and/or inefficient

class CacheEntryHash {
	public:
		typedef CacheEntry **Loc; // an address returned by find() and used in []

	public:
		CacheEntryHash(int aCapacity); // cap may be adjusted a bit
		~CacheEntryHash();             // no garbage collection

		int capacity() const { return theCapacity; }
		int count() const { return theCount; }

		bool find(const ObjId &id, Loc &loc) const;
		void add(CacheEntry *entry);

		void addAt(Loc idx, CacheEntry *entry);
		CacheEntry *delAt(Loc idx);

	protected:
		Loc theIndex;       // hash (stores pointers to real items)
		int theCapacity;    // hash capacity (in 1st level slots)
		int theCount;       // entries in the hash
};

#endif
