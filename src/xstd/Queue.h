
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_QUEUE_H
#define POLYGRAPH__XSTD_QUEUE_H



// general purpose queue

// location within the queue
template <class Item>
class QueuePlace {
	public:
		QueuePlace(): thePrev(0), theNext(0) {}

		bool isolated() const { return !theNext && !thePrev; }
		void isolate() { theNext = thePrev = 0; }

		Item *thePrev;
		Item *theNext;
};

// a queue with default FIFO interface
template <class Item, QueuePlace<Item> Item::*placePtr>
class Queue {
	public:
		Queue(): theHead(0), theCount(0) { }

		int count() const { return theCount; }
		bool empty() const { return theCount == 0; }

		inline void enqueueAfter(Item *const pos, Item *const i);
		inline Item *dequeue(Item *const i);

		Item *begin() { return firstOut(); }
		const Item *begin() const { return firstOut(); }
		Item *end() { return 0; }
		const Item *end() const { return 0; }
		inline Item *next(Item *const i);
		inline const Item *next(const Item *const i);
		inline Item *prev(Item *const i);
		inline const Item *prev(const Item *const i);

		// FIFO
		Item *firstOut() { return theHead; }
		const Item *firstOut() const { return theHead; }
		inline Item *lastIn();
		inline const Item *lastIn() const;
		void enqueue(Item *i) { enqueueAfter(lastIn(), i); }
		Item *dequeue() { return dequeue(firstOut()); }

	protected:
		// set next pointer for item
		static inline void Next(Item &i, Item *const n);
		// set previous pointer for item
		static inline void Prev(Item &i, Item *const p);

		static inline QueuePlace<Item> &Place(Item &i);
		static inline const QueuePlace<Item> &Place(const Item &i);

		Item *theHead;
		int theCount;
};

/* implementaion of in-lined methods */

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
Item *Queue<Item, placePtr>::lastIn() {
	return theHead ? Place(*theHead).thePrev : 0;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
const Item *Queue<Item, placePtr>::lastIn() const {
	return theHead ? Place(*theHead).thePrev : 0;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
void Queue<Item, placePtr>::enqueueAfter(Item *const pos, Item *const i) {
	Assert(i);
	Assert(Place(*i).isolated());
	if (pos) {
		Assert(!empty());
		Next(*i, Place(*pos).theNext);
		Next(*pos, i);
	} else {
		Assert(empty());
		theHead = i;
		Next(*i, i);
	}
	theCount++;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
Item *Queue<Item, placePtr>::dequeue(Item *const i) {
	Assert(i);
	Assert(!Place(*i).isolated());
	Assert(theCount > 0);
	Next(*Place(*i).thePrev, Place(*i).theNext);
	if (theHead == i)
		theHead = Place(*i).theNext;
	Place(*i).isolate();
	theCount--;
	if (empty())
		theHead = 0;
	return i;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
QueuePlace<Item> &Queue<Item, placePtr>::Place(Item &i) {
	return i.*placePtr;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
const QueuePlace<Item> &Queue<Item, placePtr>::Place(const Item &i) {
	return i.*placePtr;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
Item *Queue<Item, placePtr>::next(Item *const i) {
	return i && i != lastIn() ? Place(*i).theNext : 0;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
const Item *Queue<Item, placePtr>::next(const Item *const i) {
	return i && i != lastIn() ? Place(*i).theNext : 0;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
Item *Queue<Item, placePtr>::prev(Item *const i) {
	return i && i != firstOut() ? Place(*i).thePrev : 0;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
const Item *Queue<Item, placePtr>::prev(const Item *const i) {
	return i && i != firstOut() ? Place(*i).thePrev : 0;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
void Queue<Item, placePtr>::Next(Item &i, Item *const n) {
	Place(i).theNext = n;
	if (n)
		Place(*n).thePrev = &i;
}

template <class Item, QueuePlace<Item> Item::*placePtr>
inline
void Queue<Item, placePtr>::Prev(Item &i, Item *const p) {
	Place(i).thePrev = p;
	if (p)
		Place(*p).theNext = &i;
}

#endif
