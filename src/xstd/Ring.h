
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_RING_H
#define POLYGRAPH__XSTD_RING_H

#include "xstd/Array.h"
#include "xstd/Rnd.h"

// FIFO Queue of <Item>s with fixed capacity (i.e., ring)
// note dynamic resize()-ing on enqueue() does work but is disabled

template <class Item>
class Ring: protected Array<Item> {
	public:
		Ring(int aCapacity = 0): Array<Item>(aCapacity), theInOff(0), theOutOff(0) {}

		using Array<Item>::capacity;
		using Array<Item>::size;
		using Array<Item>::Value;

		int count() const { return theInOff - theOutOff; }
		bool empty() const { return theInOff <= theOutOff; }
		bool full() const { return count() >= capacity(); }
		inline const Item &top(const int off = 0) const;

		inline Item &top(const int off = 0);
		inline void enqueue(const Item i);
		inline Item dequeue();

		void reset() { theInOff = theOutOff = 0; }
		inline void resize(const int newCap);

		inline void randomize(RndGen &rng);

		inline Ring &operator =(const Ring &r);

	protected:
		inline Item &item(const int off);
		inline const Item &item(const int off) const;
		inline void check();
		inline void copyFrom(const Item *const fromItems, const int inOff, const int outOff, const int fromCapacity);
		virtual void copyOld(const Item *oldItems, int oldCapacity);

	private:
		int theInOff;
		int theOutOff;
};


/* implementaion of in-lined methods */

template <class Item>
inline
const Item &Ring<Item>::top(const int off) const {
	Must(off < count());
	return item(theOutOff+off);
}

template <class Item>
inline
Item &Ring<Item>::top(const int off) {
	Must(off < count());
	return item(theOutOff+off);
}

template <class Item>
inline
void Ring<Item>::enqueue(const Item i) {
	Must(!full());
	item(theInOff++) = i;
	check();
}

template <class Item>
inline
Item Ring<Item>::dequeue() {
	Must(!empty());
	return item(theOutOff++);
}


template <class Item>
inline
void Ring<Item>::randomize(RndGen &rng) {
	for (int i = theOutOff; i < theInOff; ++i) {
		const int j = rng(theOutOff, theInOff);
		const Item h = item(i);
		item(i) = item(j);
		item(j) = h;
	}
}

template <class Item>
inline
Ring<Item> &Ring<Item>::operator =(const Ring<Item> &r) {
	if (this->items() != r.items()) {
		reset();
		if (r.capacity() > capacity())
			resize(r.capacity());
		copyFrom(r.items(), r.theInOff, r.theOutOff, r.capacity());
	}
	return *this;
}

template <class Item>
inline
void Ring<Item>::resize(const int newCap) {
	Must(newCap >= count());
	this->grow(newCap);
}

template <class Item>
inline
Item &Ring<Item>::item(const int off) {
	// Cannot use Array::item() because Array::theCount is zero.
	return this->items()[off % capacity()];
}

template <class Item>
inline
const Item &Ring<Item>::item(const int off) const {
	// Cannot use Array::item() because Array::theCount is zero.
	return this->items()[off % capacity()];
}

template <class Item>
inline
void Ring<Item>::check() {
	Assert(theInOff > theOutOff && count() <= capacity());
	if (theOutOff >= capacity()) {
		theInOff %= capacity();
		theOutOff %= capacity();
		if (theInOff <= theOutOff)
			theInOff += capacity();
	}
}

template <class Item>
inline
void Ring<Item>::copyFrom(const Item *const fromItems, const int inOff, const int outOff, const int fromCapacity) {
	const int count = inOff - outOff;
	Must(outOff <= inOff);
	Must(count <= fromCapacity);
	Must(capacity() >= count);

	if (count > 0) {
		const int cappedInOff = inOff % fromCapacity;
		const int cappedOutOff = outOff % fromCapacity;
		if (cappedInOff > cappedOutOff)
			this->memmove(0, fromItems + cappedOutOff, count);
		else {
			this->memmove(0, fromItems + cappedOutOff,
				fromCapacity - cappedOutOff);
			this->memmove(fromCapacity - cappedOutOff, fromItems,
				cappedInOff);
		}
	}

	theOutOff = 0;
	theInOff = count;
}

template <class Item>
void Ring<Item>::copyOld(const Item *oldItems, int oldCapacity) {
	copyFrom(oldItems, theInOff, theOutOff, oldCapacity);
}

#endif
