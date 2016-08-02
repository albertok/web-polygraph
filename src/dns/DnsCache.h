
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__DNS_DNSCACHE_H
#define POLYGRAPH__DNS_DNSCACHE_H

#include <map>
#include "xstd/String.h"
#include "xstd/NetAddr.h"
#include "xstd/LruQueue.h"

class DnsCacheSym;

// cached information for a given DNS name
class DnsCacheEntry {
	public:
		typedef String Key;
		NetAddr ip; // IP address
		LruQueue<Key>::Position lruPos; // entry position in the LRU queue
};

// manages a DNS lookup cache
class DnsCache {
	public:
		// cache entries indexed by query names
		typedef std::map<DnsCacheEntry::Key, DnsCacheEntry> Storage;

	public:
		static DnsCache *Create(const DnsCacheSym *cfg);
		static void Destroy(DnsCache *cache);

		void use() { ++theUseLvl; }
		void abandon();
		bool find(const DnsCacheEntry::Key &addr, NetAddr &ip);
		void insert(const DnsCacheEntry::Key &addr, const NetAddr &ip);
		void clear();

		const String sharingGroup;

		// maximum number of entries we are allowed to cache; 0: no limit
		const Storage::size_type capacity;

	private:
		DnsCache(const String &aSharingGroup, const int aCapacity);

		int theUseLvl; // number of busy robots using us
		int theSharingLvl; // number of robots pointing to us

		Storage theStorage;
		// cache eviction policy
		typedef LruQueue<DnsCacheEntry::Key> Index;
		Index theIndex;

		// caches indexed by their sharing group
		typedef std::map<String, DnsCache*> Caches;
		static Caches TheSharedCaches; // for sharing caches among DnsMgrs
};

#endif
