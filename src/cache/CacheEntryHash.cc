
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>
#include "xstd/h/string.h"

#include "cache/CacheEntry.h"
#include "cache/CacheEntryHash.h"


CacheEntryHash::CacheEntryHash(int aCapacity): theCount(0) {
	theCapacity = (aCapacity + aCapacity/3 + 7) | 1;
	theIndex = new CacheEntry*[theCapacity];
	memset(theIndex, 0, sizeof(CacheEntry*)*theCapacity);
}

CacheEntryHash::~CacheEntryHash() {
	delete[] theIndex;
}

bool CacheEntryHash::find(const ObjId &id, Loc &loc) const {
	// bucket position
	loc = theIndex + (id.hash() % theCapacity);

	// search the bucket
	while (*loc && (*loc)->id() < id) loc = &(*loc)->nextInIdx;

	return *loc && (*loc)->id() == id;
}

void CacheEntryHash::addAt(Loc loc, CacheEntry *e) {
	Assert(loc && e);
	e->nextInIdx = *loc;
	*loc = e;
	theCount++;
}

void CacheEntryHash::add(CacheEntry *e) {
	Loc loc;
	Assert(!find(e->id(), loc));
	addAt(loc, e);
}

CacheEntry *CacheEntryHash::delAt(Loc loc) {
	Assert(*loc);
	CacheEntry *e = *loc;
	*loc = e->nextInIdx;
	e->nextInIdx = 0;
	theCount--;
	return e;
}
