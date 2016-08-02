
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/string.h"

#include "tools/IntIntHash.h"
#include "xstd/gadgets.h"


IntIntHash::IntIntHash(int aCapacity): theHashCap(0) {
	theHashCap = (aCapacity + aCapacity/3 + 7) | 1;
	theIndex = new IntIntHashItem*[theHashCap];
	memset(theIndex, 0, sizeof(IntIntHashItem*)*theHashCap);
}

IntIntHash::~IntIntHash() {
	delete[] theIndex;
}

double IntIntHash::utilp() const {
	return Percent(theHashCnt, theHashCap);
}

bool IntIntHash::find(int key, Loc &loc) const {
	Assert(key);

	loc = theIndex + hashIdx(key);

	// search the chain
	while (*loc && (*loc)->key < key) loc = &(*loc)->next;

	return *loc && (*loc)->key == key;
}

void IntIntHash::addAt(Loc loc, int key, int val) {
	Assert(key);

	IntIntHashItem *i = getNewItem();
	i->next = *loc;
	i->key = key;
	i->val = val;
	*loc = i;

	if (!i->next && theIndex <= loc && loc < theIndex+theHashCap) {
		theHashCnt++;
		Assert(theHashCnt <= theHashCap);
	}
}

void IntIntHash::delAt(Loc loc) {
	Assert(*loc && (*loc)->key);

	IntIntHashItem *i = *loc;
	*loc = i->next;
	putOldItem(i);

	if (!*loc && theIndex <= loc && loc < theIndex+theHashCap) {
		theHashCnt--;
		Assert(theHashCnt >= 0);
	}
}
