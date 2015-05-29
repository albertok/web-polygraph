
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__TOOLS_INTINTHASH_H
#define POLYGRAPH__TOOLS_INTINTHASH_H

#include <limits.h>

// a simple and efficient hash indexed on non-zero 'int'
// the hash stores an 'int' value for each entry
// we will make it more "generic" iff needed

// hash item
struct IntIntHashItem {
	IntIntHashItem *next; // next item in chain
	int key; // position is empty if key is zero
	int val; // initialized to zero

	IntIntHashItem(): next(0), key(0), val(0) {}
};

class IntIntHash {
	public:
		typedef IntIntHashItem **Loc; // an address returned by find() and used in []

	public:
		IntIntHash(int aCapacity); // may be adjusted a bit
		~IntIntHash();

		double utilp() const;

		bool find(int key, Loc &loc) const;
		void addAt(Loc idx, int key, int val);
		void delAt(Loc idx);

		int operator ()(Loc loc) const { return (*loc)->key; }
		int &operator [](Loc loc) { return (*loc)->val; }
		int operator [](Loc loc) const { return (*loc)->val; }

	protected:
		inline int hashIdx(int key) const;

		IntIntHashItem *getNewItem() { return new IntIntHashItem; }
		void putOldItem(IntIntHashItem *i) { delete i; }

	protected:
		Loc theIndex;         // hash (stores pointers to real items)
		int theHashCap;       // hash capacity
		int theHashCnt;       // active pointers in the hash
};


/* inlined methods */

inline
int IntIntHash::hashIdx(int key) const {
	if (key < 0) key += INT_MAX;
	return key % theHashCap;
}

#endif
