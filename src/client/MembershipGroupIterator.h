
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_MEMBERSHIPGROUPITERATOR_H
#define POLYGRAPH__CLIENT_MEMBERSHIPGROUPITERATOR_H

#include "xstd/Interval.h"

class MembershipMap;

// iterates over group names for a specified group ID range
class MembershipGroupIterator {
	public:
		MembershipGroupIterator(const MembershipMap *aMap, Interval &aRange);

		bool atEnd() const { return !theRange.has(thePos); }
		operator void*() const { return atEnd() ? 0 : (void*)this; }
		String operator *() const;

		MembershipGroupIterator &operator ++();

	protected:
		void sync() {}
		void step() { ++thePos; }

	protected:
		const MembershipMap *theMembership;
		const Interval theRange;
		int thePos;
};

#endif
