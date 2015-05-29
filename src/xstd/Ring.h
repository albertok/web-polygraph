
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

		int capacity() const { return this->theCapacity; }
		int count() const { return this->theInOff - this->theOutOff; }
		int size() const { return Array<Item>::size(); }
		bool empty() const { return this->theInOff <= this->theOutOff; }
		bool full() const { return count() >= this->theCapacity; }
		const Item &top(int off = 0) const { return item((this->theOutOff+off) % this->theCapacity); }

		Item &top(int off = 0) { return item((this->theOutOff+off) % this->theCapacity); }
		void enqueue(Item i) { Assert(!full()); item(this->theInOff++ % this->theCapacity) = i; check(); }
		Item dequeue() { return item(this->theOutOff++ % this->theCapacity); }

		void reset() { this->theInOff = this->theOutOff = 0; }
		inline void resize(int aCap);

		inline void randomize(RndGen &rng);

	protected:
		inline void check();

	protected:
		int theInOff;
		int theOutOff;
};


/* implementaion of in-lined methods */

template <class Item>
inline
void Ring<Item>::randomize(RndGen &rng) {
	for (int i = this->theOutOff; i < this->theInOff; ++i)
		Array<Item>::swap(i % this->theCapacity,
			rng(this->theOutOff, this->theInOff) % this->theCapacity);
}

template <class Item>
inline
void Ring<Item>::resize(int aCap) {
	const int oldCap = this->theCapacity;
	Item *oldItems = this->theItems;
	this->theItems = 0;

	this->grow(aCap);

	if (oldItems) {
		if (count()) {
			const int old_hi = this->theInOff > oldCap ? oldCap : this->theInOff;
			const int gap = this->theCapacity - old_hi;
			const int new_low = this->theOutOff + gap;
			if (old_hi > this->theOutOff)
				this->memmove(new_low, oldItems+this->theOutOff, old_hi-this->theOutOff);
			if (this->theInOff > oldCap)
				this->memmove(0, oldItems, this->theInOff-oldCap);
			this->theInOff += gap;
			this->theOutOff += gap;
		}
		delete[] oldItems;
	}
}

template <class Item>
inline
void Ring<Item>::check() {
	Assert(this->theInOff > this->theOutOff && count() <= this->theCapacity);
	if (this->theOutOff >= this->theCapacity) {
		this->theInOff %= this->theCapacity;
		this->theOutOff %= this->theCapacity;
		if (this->theInOff <= this->theOutOff)
			this->theInOff += this->theCapacity;
	}
}

#endif
