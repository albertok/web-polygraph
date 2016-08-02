
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_HEAP_H
#define POLYGRAPH__XSTD_HEAP_H

#include "xstd/Array.h"

// Heap "tree" adopted with modifications from
// "Algorithms in C", Sedgewick, Addison-Wesley, 1990, ISBN 0-201-51425-7

template <class Item>
class Heap: protected Array<Item> {
	public:
		inline Heap(const int capacity = 2);

		int count() const { return theCnt; }
		bool empty() const { return theCnt <= 0; }

		Item &top() { return this->item(1); }
		const Item &top() const { return this->item(1); }
		Item &at(int idx) { return this->item(idx+1); }

		void add(Item v) { this->append(v); floatUp(++theCnt); }
		void skip() { top() = this->pop(); --theCnt; sinkDown(1); }
		Item shift() { const Item v = top(); skip(); return v; }

	protected:
		inline void floatUp(int k);
		inline void sinkDown(int k);

	protected:
		int theCnt;  // theCount-1
};


/* implementaion of in-lined methods */

template <class Item>
inline
Heap<Item>::Heap(const int capacity): Array<Item>(capacity), theCnt(0) {
	this->resize(1);
}

template <class Item>
inline
void Heap<Item>::floatUp(int k) {
	const Item v = this->item(k);
	for (int p = k/2; p && this->item(p) > v; k = p, p /= 2)
		this->item(k) = this->item(p);
	this->item(k) = v;
}

template <class Item>
inline
void Heap<Item>::sinkDown(int k) {
	if (empty())
		return;

	const int half = theCnt/2;
	const Item v = this->item(k);
	while (k <= half) {
		// smallest of the (at most) two kids
		int j = k*2;
		if (j < theCnt && this->item(j+1) < this->item(j))
			j++;
		if (v < this->item(j))
			break;
		this->item(k) = this->item(j);
		k = j;
	}
	this->item(k) = v;
}

#endif
