
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
		typedef Item Value;

	public:
		Array(int aCapacity = 0): theItems(0), theCapacity(0), theCount(0) { grow(aCapacity); }
		virtual ~Array() { delete[] theItems; }

		bool full() const { return theCount >= theCapacity; }
		bool empty() const { return !theCount; }
		int capacity() const { return theCapacity; }
		int count() const { return theCount; }
		int size() const { return theCapacity*sizeof(Item); }

		inline Item &item(int idx);
		inline const Item &item(int idx) const;
		Item &operator [](int idx) { return theItems[idx]; }
		const Item &operator [](int idx) const { return theItems[idx]; }
		Item &last(int off = 0) { return item(count()-1-off); }
		const Item &last(int off = 0) const { return item(count()-1-off); }

		void reset() { theCount = 0; }
		void clear() { clear(0, theCapacity); }
		void stretch(int n) { if (n > theCapacity) grow(n); }
		void resize(int n) { stretch(n); clear(count(), n); theCount = n; }

		inline void append(const Item &i);
		inline Item shift() { const Item i = item(0); shift(1); return i; }
		inline void shift(int n);
		void push(const Item &i) { append(i); }
		Array<Item> &operator <<(const Item &i) { append(i); return *this; }
		void pop(int n) { Assert(theCount >= n); theCount -= n; }
		Item pop() { const Item i = last(); --theCount; return i; }
		Array<Item> &operator >>(Item &i) { i = pop(); return *this; }
		inline void put(const Item &i, const int idx);
		inline void insert(const Item &i, const int idx);
		inline void swap(const int i, const int j);
		void eject(int i) { item(i) = last(); --theCount; }

		const Item *items() const { return theItems; }
		Item *items() { return theItems; }

		inline Array &operator =(const Array &arr);

		inline bool find(const Item &i, int &idx) const; // first matching item
		inline bool findOther(const Item &i, int &idx) const; // first different item

	protected:
		Array(const Array &) {} // no support for silent copying

		void grow() { grow(theCapacity <= 0 ? 16 : theCapacity*2); }
		inline void grow(int newCap);
		virtual void copyOld(const Item *oldItems, int oldCapacity);
		inline void clear(int offFrom, int offTo);
		inline void memmove(int off, const Item *from, int cnt);

	private:
		Item *theItems;
		int theCapacity;
		int theCount;
};

/* array of pointers to objects */
template <class Item>
class PtrArray: public Array<Item> {
	public:
		virtual ~PtrArray();
};


/* implementaion of in-lined methods */

template <class Item>
inline
Item &Array<Item>::item(const int idx) {
	Assert(0 <= idx && idx < count());
	return theItems[idx];
}

template <class Item>
inline
const Item &Array<Item>::item(const int idx) const {
	Assert(0 <= idx && idx < count());
	return theItems[idx];
}

template <class Item>
inline
void Array<Item>::put(const Item &i, const int idx) {
	if (idx >= theCount)
		resize(idx+1);
	item(idx) = i;
}

template <class Item>
inline
void Array<Item>::insert(const Item &i, const int idx) {
	stretch(count()+1);
	memmove(idx+1, items()+idx, count()-idx);
	++theCount;
	item(idx) = i;
}

template <class Item>
inline
void Array<Item>::swap(const int i, const int j) {
	const Item h = item(i);
	item(i) = item(j);
	item(j) = h;
}

template <class Item>
inline
void Array<Item>::append(const Item &i) {
	if (theCount >= theCapacity)
		grow();
	++theCount;
	last() = i;
}

// efficient, but not very generic
template <class Item>
inline
void Array<Item>::grow(int newCap) {
	Assert(newCap >= count());
	const Item *const oldItems = theItems;
	const int oldCapacity = capacity();
	theItems = new Item[newCap];
	theCapacity = newCap;
	if (oldItems)
		copyOld(oldItems, oldCapacity);
	delete[] oldItems;
}

template <class Item>
void Array<Item>::copyOld(const Item *oldItems, int oldCapacity) {
	if (!empty())
		memcpy(theItems, oldItems, count()*sizeof(Item));
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
}

template <class Item>
inline
void Array<Item>::clear(int offFrom, int offTo) {
	if (offFrom < theCapacity && offFrom < offTo) {
		if (offTo > theCapacity)
			offTo = theCapacity;
		memset(theItems + offFrom, 0, (offTo - offFrom)*sizeof(Item));
	}
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
bool Array<Item>::find(const Item &i, int &idx) const {
	for (idx = 0; idx < count(); ++idx)
		if (item(idx) == i)
			return true;
	return false;
}

template <class Item>
inline
bool Array<Item>::findOther(const Item &i, int &idx) const {
	for (idx = 0; idx < count(); ++idx)
		if (item(idx) != i)
			return true;
	return false;
}

template <class Item>
PtrArray<Item>::~PtrArray() {
	while (this->count())
		delete this->pop();
}


#endif
