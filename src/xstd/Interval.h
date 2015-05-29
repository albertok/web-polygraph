
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_INTERVAL_H
#define POLYGRAPH__XSTD_INTERVAL_H

#include "xstd/gadgets.h" /* just for Max */

// beg <= x < end
class Interval {
	public:
		Interval(): theBeg(0), theEnd(0) {}
		Interval(int aBeg, int anEnd): theBeg(aBeg), theEnd(anEnd) {}

		bool empty() const { return theBeg >= theEnd; }
		operator void*() const { return empty() ? 0 : (void*)this; }

		int beg() const { return theBeg; }
		int end() const { return theEnd; }
		int size() const { return theBeg < theEnd ? theEnd - theBeg : 0; }

		bool has(int point) const { return theBeg <= point && point < theEnd; }

		void begAt(int aBeg) { theBeg = aBeg; }
		void endAt(int anEnd) { theEnd = anEnd; }
		inline Interval exceptHead(const Interval &i) const;

	protected:
		int theBeg;
		int theEnd;
};

inline
Interval Interval::exceptHead(const Interval &i) const {
	const int newBeg = Max(beg(), i.end());
	if (newBeg < end())
		return Interval(newBeg, end());
	else
		return Interval(end(), end());
}

#endif
