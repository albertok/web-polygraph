
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_HISTORY_H
#define POLYGRAPH__XSTD_HISTORY_H

#include "xstd/Array.h"
#include "xstd/gadgets.h"

// History log: a circular buffer; new entries overwrite old ones
// all external coordinates are relative to "now" or last entry
// for simplicity, assume that at most MAX_INT items are inserted
template <class Item>
class History {
	public:
		History(int aCapacity = 0): theLog(aCapacity), theOffset(0) {}

		void capacity(int aCapacity) { Assert(!theOffset); theLog.stretch(aCapacity); }

		int depth() const { return Min(theOffset, theLog.capacity()); }
		Item at(int off) const { return theLog[(theOffset - off) % theLog.capacity()]; }
		Item operator [](int off) const { return at(off); } 

		void insert(Item i) { theLog[++theOffset % theLog.capacity()] = i; }

	protected:
		Array<Item> theLog;
		int theOffset;      // beginning of a buffer
};

#endif
