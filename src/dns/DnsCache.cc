
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits>

#include "xstd/h/iomanip.h"
#include "xstd/String.h"
#include "dns/DnsCache.h"
#include "pgl/DnsCacheSym.h"
#include "runtime/LogComment.h"

DnsCache::Caches DnsCache::TheSharedCaches;

// a hack to debug caching of the first few DNS caching actions
static
bool DebugCache() {
	static unsigned int limit = 10;
	static unsigned int attempts = 0;
	if (++attempts > limit)
		return false;

	if (attempts == limit) {
		Comment(7) << "fyi: suspending DNS cache debugging for the next " <<
			(std::numeric_limits<unsigned int>::max() - attempts) <<
			" caching events" << endc;
		return false;
	}

	return true;
}

DnsCache::DnsCache(const String &aSharingGroup, const int aCapacity):
	sharingGroup(aSharingGroup),
	capacity(static_cast<Storage::size_type>(aCapacity)),
	theUseLvl(0),
	theSharingLvl(1) {
	ShouldUs(capacity > 0);
}

void DnsCache::abandon() {
	--theUseLvl;
	ShouldUs(theUseLvl >= 0);
	if (!theUseLvl)
		clear();
}

bool DnsCache::find(const DnsCacheEntry::Key &addr, NetAddr &ip) {
	Storage::iterator it = theStorage.find(addr);
	if (it == theStorage.end()) {
		if (DebugCache())
			Comment(5) << "fyi: DNS cache miss: " << addr << endc;
		return false;
	}
	ip = it->second.ip;
	it->second.lruPos = theIndex.touchAt(it->second.lruPos);
	if (DebugCache())
		Comment(5) << "fyi: DNS cache hit:  " << addr << " is " << ip << endc;
	return true;
}

void DnsCache::insert(const DnsCacheEntry::Key &addr, const NetAddr &ip) {
	if (!ShouldUs(capacity > 0))
		return;

	Storage::iterator it = theStorage.find(addr);
	if (it != theStorage.end()) {
		it->second.ip = ip;
		it->second.lruPos = theIndex.touchAt(it->second.lruPos);
		if (DebugCache() && it->second.ip != ip)
			Comment(5) << "fyi: DNS cache update: " << addr << " is " << ip << endc;
		return;
	}

	if (theStorage.size() >= capacity) {
		it = theStorage.find(theIndex.eraseVictim());
		Assert(it != theStorage.end());
		theStorage.erase(it);
	}

	DnsCacheEntry entry;
	entry.ip = ip;
	entry.lruPos = theIndex.insert(addr);
	theStorage.insert(std::make_pair(addr, entry));

	if (DebugCache())
		Comment(5) << "fyi: DNS cache add:  " << addr << " is " << ip << endc;
}

void DnsCache::clear() {
	theStorage.clear();
	theIndex.clear();
}

DnsCache *DnsCache::Create(const DnsCacheSym *cfg) {
	int cap = 0;
	if (!cfg || !cfg->capacity(cap) || !cap)
		return 0; // undefined/zero capacity means no caching (default)
	if (cap < 0)
		cerr << cfg->loc() << "error: DnsCache::capacity cannot be negative; got: " << cap << endl << xexit;

	const String sharingGroup = cfg->sharingGroup();
	if (!sharingGroup) // this cache is dedicated to a single DnsMgr
		return new DnsCache(sharingGroup, cap);

	const Caches::iterator it = TheSharedCaches.find(sharingGroup);
	if (it != TheSharedCaches.end() && ShouldUs(it->second)) {
		++it->second->theSharingLvl;
		ShouldUs(static_cast<Storage::size_type>(cap) == it->second->capacity);
		return it->second;
	}

	DnsCache *cache = new DnsCache(sharingGroup, cap);
	TheSharedCaches.insert(std::make_pair(sharingGroup, cache));
	return cache;
}

void DnsCache::Destroy(DnsCache *cache) {
	if (!cache)
		return;

	if (--cache->theSharingLvl) {
		ShouldUs(cache->sharingGroup);
		return; // somebody is still pointing to us
	}

	if (cache->sharingGroup) {
		const Caches::iterator it = TheSharedCaches.find(cache->sharingGroup);
		if (ShouldUs(it != TheSharedCaches.end())) {
			ShouldUs(it->second == cache);
			TheSharedCaches.erase(it);
		}
	}

	delete cache;
}
