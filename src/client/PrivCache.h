
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_PRIVCACHE_H
#define POLYGRAPH__CLIENT_PRIVCACHE_H

#include "xstd/Time.h"
#include "xstd/Array.h"
#include "xstd/Ring.h"

class ObjId;

// an LRU-driven, slightly imprecise, memory inefficient cache
class PrivCache {
	public:
		PrivCache(int aCapacity = 0);

		void capacity(int aCap);

		void clear();

		bool hasOid(const ObjId &oid) const;
		bool loadOid(const ObjId &oid);
		bool storeOid(const ObjId &oid);
		bool purgeOid(const ObjId &oid);

	protected:
		void addOid(const ObjId &oid);
		void delOid(const ObjId &oid);

		int hash(const ObjId &oid) const;
		int index(int hash) const;
		void delAt(int idx);
		void purgeOne();

	protected:
		class Entry {
			public:
				Entry(): key(0) {}

				operator void *() const { return key > 0 ? (void*)-1 : (void*)0; }
				bool operator !() const { return key <= 0; }

			public:
				int key; // currently same as oid.hash()
		};

		typedef int HistoryItem; // currently same as Entry::key

	protected:
		Ring<HistoryItem> theHistory;       // index into a hash
		Array<PrivCache::Entry> theHash;
		int theEntryCount;
		int theHitCount;
		int theMissCount;
};

#endif
