
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_FUTURE_H
#define POLYGRAPH__XSTD_FUTURE_H

#include "xstd/Array.h"

// "Future plan": a circular buffer; new entries are inserted
// at random positions relative to "now"; "now" or "next step"
// is at position "zero"
// for simplicity, assume that at most MAX_INT items are inserted
// over the life time of an object
template <class Item>
class Future {
	public:
		Future(int aCapacity = 0): theLog(aCapacity), theOffset(0) {}

		void capacity(int aCapacity) { Assert(!theOffset); theLog.stretch(aCapacity); }
		int capacity() const { return theLog.capacity(); }

		const Item &operator [](int off) const { return theLog[(theOffset + off) % theLog.capacity()]; }
		Item &operator [](int off) { return theLog[(theOffset + off) % theLog.capacity()]; }
		void shift(Item i) { theLog[theOffset++ % theLog.capacity()] = i; }

	protected:
		Array<Item> theLog;
		int theOffset;      // "now" or the start of a plan
};

#endif
