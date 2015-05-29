
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CACHE_CACHE_H
#define POLYGRAPH__CACHE_CACHE_H

#include "xstd/BigSize.h"
#include "runtime/Farm.h"
#include "cache/CacheEntry.h"

class ObjId;
class Client;
class CacheEntryHash;
class CachePolicy;
class CacheSym;
class DistrPoint;
class ReqHdr;
class RepHdr;

// common interface for cache readers and writers
class CacheUser {
	public:
		friend class DistrPoint;

	public:
		CacheUser();
		virtual ~CacheUser();

		virtual void reset();

	protected:
		DistrPoint *theDistrPoint;
};

// interface for reading from the cache
class CacheReader: public CacheUser {
	public:
		virtual const ReqHdr *origReqHdrs() const = 0;

		virtual void noteWriterLeft() = 0;
		virtual void noteCacheReady() = 0;
};

// interface for reading from the cache
class CacheWriter: public CacheUser {
	public:
		virtual const RepHdr *origRepHdrs() const = 0;

		virtual void noteReaderLeft() = 0;
};

// maintater of entries handled by the proxy
class Cache {
	public:
		friend class CachePolicy;

	public:
		Cache();
		~Cache();

		void configure(const CacheSym *cfg);
		void client(Client *client);

		BigSize capacity() const { return theCapacity; }
		BigSize space() const { return theCapacity - theSize; }
		BigSize size() const { return theSize; }

		CacheEntry *cached(const ObjId &id) const;

		Client *client() { return theClient; }

		bool cacheEntry(CacheEntry *e);
		bool purgeEntry(CacheEntry *e);

		DistrPoint *addReader(const ObjId &id, CacheReader *r);
		DistrPoint *addWriter(const ObjId &id, CacheWriter *w);

		// called by a stray cache entry
		void noteStrayEntry(CacheEntry *e);

	protected:
		DistrPoint *getDistrPoint(const ObjId &id, bool canCreate);

	protected:
		static ObjFarm<CacheEntry> TheEntries;

		CacheEntryHash *theIndex;
		CachePolicy *thePolicy;   // removal policy
		Client *theClient;

		BigSize theCapacity;      // maximum capacity
		BigSize theSize;          // current size
};

#endif
