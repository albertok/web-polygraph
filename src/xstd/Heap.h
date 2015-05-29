
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
		Heap(int aCapacity = 2): Array<Item>(aCapacity), theCnt(0) { this->theCount = 1; }

		int count() const { return theCnt; }
		bool empty() const { return theCnt <= 0; }

		Item &top() { return this->theItems[1]; }
		const Item &top() const { return this->theItems[1]; }
		Item &at(int idx) { return this->theItems[idx+1]; }

		void add(Item v) { append(v); floatUp(++theCnt); }
		void skip() { this->theItems[1] = this->theItems[theCnt--]; this->theCount--; sinkDown(1); }
		Item shift() { const Item v = this->theItems[1]; skip(); return v; }

		void resize(int aCap) { Array<Item>::stretch(aCap); }

	protected:
		inline void floatUp(int k);
		inline void sinkDown(int k);

	protected:
		int theCnt;  // theCount-1
};


/* implementaion of in-lined methods */

template <class Item>
inline
void Heap<Item>::floatUp(int k) {
	const Item v = this->theItems[k];
	for (int p = k/2; p && this->theItems[p] > v; k = p, p /= 2)
		this->theItems[k] = this->theItems[p];
	this->theItems[k] = v;
}

template <class Item>
inline
void Heap<Item>::sinkDown(int k) {
	const int half = theCnt/2;
	const Item v = this->theItems[k];
	while (k <= half) {
		// smallest of the (at most) two kids
		int j = k*2;
		if (j < theCnt && this->theItems[j+1] < this->theItems[j])
			j++;
		if (v < this->theItems[j])
			break;
		this->theItems[k] = this->theItems[j];
		k = j;
	}
	this->theItems[k] = v;
}

#endif
