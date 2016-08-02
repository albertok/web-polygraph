
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_LRU_QUEUE_H
#define POLYGRAPH__XSTD_LRU_QUEUE_H

#include <map>
#include "xstd/h/string.h"
#include "xstd/Assert.h"


// a Least Recently Used queue of Items
template <class Item>
class LruQueue {
	public:
		// LRU order is based on "time of access", but absolute time values do
		// not really matter because LRU just needs to know which entry was
		// accessed earlier than others. We measure time of access in relative
		// terms, incrementing our "clock" when an item is added or updated.
		// XXX: Clock overflows are not yet supported.
		typedef uint64_t Position; // a.k.a. "relative clock"

		void clear() { items.clear(); }

		// adds a new item to the queue, returning its position
		Position insert(const Item &e);
		// requeues an old item, moving it to the end of the queue
		Position touchAt(const Position &idx);
		// removes a given item from the queue
		Item eraseAt(const Position &idx);
		// removes the least recently used item from the queue and returns it
		Item eraseVictim();

	protected:
		Position currentTime() const { return items.empty() ? 0 : (items.rbegin()->first+1); }

	private:
		typedef std::map<Position, Item> Items;
		Items items;
};


/* implementaion of in-lined methods */

template <class Item>
inline
typename LruQueue<Item>::Position LruQueue<Item>::insert(const Item &e) {
	const Position now = currentTime();
	items.insert(std::make_pair(now, e));
	return now;
}

template <class Item>
inline
typename LruQueue<Item>::Position LruQueue<Item>::touchAt(const Position &idx) {
	const Item item = eraseAt(idx);
	return insert(item);
}

template <class Item>
inline
Item LruQueue<Item>::eraseAt(const Position &idx) {
	typename Items::iterator it = items.find(idx);
	Assert(it != items.end());
	const Item item = it->second;
	items.erase(it);
	return item;
}

template <class Item>
inline
Item LruQueue<Item>::eraseVictim() {
	Assert(!items.empty());
	typename Items::iterator it = items.begin();
	const Item item = it->second;
	items.erase(it);
	return item;
}

#endif
