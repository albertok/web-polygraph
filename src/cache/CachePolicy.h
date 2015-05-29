
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CACHE_CACHEPOLICY_H
#define POLYGRAPH__CACHE_CACHEPOLICY_H

#include "xstd/Queue.h"
#include "cache/CacheEntry.h"

class Cache;

// we will generalize the policy class iff needed

class CachePolicy {
	public:
		CachePolicy(Cache *aCache);
		~CachePolicy();

		bool canAdmit(CacheEntry *e) { return e != 0; }
		void noteAdmitted(CacheEntry *e) { theQueue.enqueue(e); }
		void notePurged(CacheEntry *e) { theQueue.dequeue(e); }

		CacheEntry *nextVictim() { return theQueue.firstOut(); }

	protected:
		Queue<CacheEntry, &CacheEntry::cacheEntries> theQueue;
		Cache *theCache;
};

#endif
