
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_ARRAY_H
#define POLYGRAPH__XSTD_ARRAY_H

#include "xstd/h/string.h"

#include "xstd/Assert.h"

/*
 * Array is an array of <Item> items with an unlimited capacity.
 * Note: for optimization purposes, we mem-copy Items if needed.
 * In general, we cannot keep objects with non-default destructors
 * inside Array. Besides allocated memory is zeroed before use.
 * This means that Array also cannot be used for classes with
 * constructors that do something other than zero their members.
 * In particular, Array of Arrays leads to (zero-size) memory
 * leaks.
 */
template <class Item>
class Array {
	public:
		Array(int aCapacity = 0): theItems(0), theCapacity(0), theCount(0) { grow(aCapacity); }
		~Array() { delete[] theItems; }

		bool full() const { return theCount >= theCapacity; }
		bool empty() const { return !theCount; }
		int capacity() const { return theCapacity; }
		int count() const { return theCount; }
		int size() const { return theCapacity*sizeof(Item); }

		Item &item(int idx) { return theItems[idx]; }
		const Item &item(int idx) const { return theItems[idx]; }
		Item &operator [](int idx) { return item(idx); }
		const Item &operator [](int idx) const { return item(idx); }
		Item &last(int off = 0) { return theItems[theCount-1-off]; }
		const Item &last(int off = 0) const { return theItems[theCount-1-off]; }

		void reset() { theCount = 0; }
		void stretch(int n) { if (n > theCapacity) grow(n); }
		void count(int aCount) { Assert(aCount <= theCapacity); theCount = aCount; }

		inline void append(const Item &i);
		inline Item shift() { Item i(theItems[0]); shift(1); return i; }
		inline void shift(int n);
		void push(const Item &i) { append(i); }
		Array<Item> &operator <<(const Item &i) { append(i); return *this; }
		void pop(int n) { Assert(theCount >= n); theCount -= n; }
		Item pop() { Assert(theCount > 0); return theItems[--theCount]; }
		Array<Item> &operator >>(Item &i) { i = pop(); return *this; }
		void put(const Item &i, int idx) { stretch(idx+1); theItems[idx] = i; if (theCount <= idx) theCount = idx+1; }
		void insert(const Item &i, int idx) { stretch(count()+1); memmove(idx+1, items()+idx, count()-idx); theItems[idx] = i; ++theCount; }
		void swap(int i, int j) { const Item h = theItems[i]; theItems[i] = theItems[j]; theItems[j] = h; }
		void eject(int i) { theItems[i] = theItems[--theCount]; }

		inline void memset(int v, int off = 0);
		inline void memmove(int off, const Item *from, int cnt);
		const Item *items() const { return theItems; }
		Item *items() { return theItems; }

		inline Array &operator =(const Array &arr);

	protected:
		Array(const Array &) {} // no support for silent copying

		void grow() { grow(theCapacity <= 0 ? 16 : theCapacity*2); }
		inline void grow(int newCap);

	protected:
		Item *theItems;
		int theCapacity;
		int theCount;
};

/* searchable array */
template <class Item>
class SchArray: public Array<Item> {
	public:
		SchArray(int aCapacity = 0): Array<Item>(aCapacity) {}
		inline bool find(const Item &i, int &idx) const; // first matching item
		inline bool findOther(const Item &i, int &idx) const; // first different item
};

/* array of pointers to objects */
template <class Item>
class PtrArray: public Array<Item> {
	public:
		inline ~PtrArray();
};


/* implementaion of in-lined methods */

template <class Item>
inline
void Array<Item>::append(const Item &item) {
    if (theCount >= theCapacity)
		grow();
    theItems[theCount++] = item;
}

// efficient, but not very generic
template <class Item>
inline
void Array<Item>::grow(int newCap) {
	Assert(newCap >= theCount);
	Item *oldItems = theItems;
	theItems = new Item[newCap];
	if (oldItems && theCount)
		memcpy(theItems, oldItems, theCount*sizeof(Item));
	delete[] oldItems;
	theCapacity = newCap;
	memset(0, theCount);
}

// discards first n items
template <class Item>
inline
void Array<Item>::shift(int n) {
	
	if (theCount > n) {
		theCount -= n;
		memmove(0, theItems + n, theCount);
	} else
		theCount = 0;
	memset(0, theCount);
}

template <class Item>
inline
void Array<Item>::memset(int v, int off) {
	if (off < theCapacity)
		::memset(theItems+off, v, (theCapacity-off)*sizeof(Item));
}

template <class Item>
inline
void Array<Item>::memmove(int off, const Item *from, int cnt) {
	if (cnt > 0)
		::memmove(theItems+off, from, cnt*sizeof(Item));
}

template <class Item>
inline
Array<Item> &Array<Item>::operator =(const Array<Item> &arr) {
	if (items() != arr.items()) {
		reset();
		stretch(arr.count());
		for (int idx = 0; idx < arr.count(); ++idx)
			append(arr[idx]);
	}
	return *this;
}

template <class Item>
inline
bool SchArray<Item>::find(const Item &i, int &idx) const {
	for (idx = 0; idx < this->theCount; ++idx)
		if (this->item(idx) == i)
			return true;
	return false;
}

template <class Item>
inline
bool SchArray<Item>::findOther(const Item &i, int &idx) const {
	for (idx = 0; idx < this->theCount; ++idx)
		if (this->item(idx) != i)
			return true;
	return false;
}

template <class Item>
inline
PtrArray<Item>::~PtrArray() {
	while (this->count())
		delete this->pop();
}


#endif
