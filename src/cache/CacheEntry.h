
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CACHE_CACHEENTRY_H
#define POLYGRAPH__CACHE_CACHEENTRY_H

#include "xstd/Queue.h"
#include "base/ObjId.h"

class Cache;
class DistrPoint;

// a central point for information about all objects handled by the cache

class CacheEntry {
	public:
		friend class DistrPoint;

	public:
		CacheEntry();
		CacheEntry(const ObjId &anId);
		~CacheEntry();

		void reset();

		const ObjId &id() const { return theId; }
		DistrPoint *distrPoint() { return theDistrPoint; }
		Size objSize() const { return theObjSize; }
		bool cached() const { return !cacheEntries.isolated(); }
		bool stray() const { return cacheEntries.isolated() && !theDistrPoint; }

		DistrPoint *startDistributing(Cache *cache);
		void stopDistributing(DistrPoint *dp);

		void id(const ObjId &anId) { theId = anId; }
		void objSize(Size sz) { theObjSize = sz; }

	public: /* XXX */
		QueuePlace<CacheEntry> cacheEntries;
		CacheEntry *nextInIdx; // next entry in cache index

	protected:
		DistrPoint *theDistrPoint;   // for active entries only
		ObjId theId;
		Size theObjSize;             // negative if yet unknown

};

#endif
