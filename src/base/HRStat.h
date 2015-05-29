
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_HRSTAT_H
#define POLYGRAPH__BASE_HRSTAT_H

#include "base/TmSzStat.h"

// handy for keeping track of hit ratios, cachability ratios, etc.
// "hit" and "miss" classes must be disjoint
class HRStat {
	public:
		HRStat();

		void reset();
		OLog &store(OLog &log) const;
		ILog &load(ILog &);

		bool active() const;
		bool sane() const;

		inline void record(const Time &tm, Size sz, bool hit);

		const TmSzStat &hits() const { return theHits; }
		const TmSzStat &misses() const { return theMisses; }
		TmSzStat xacts() const;

		double dhr() const;
		double bhr() const;
		double dhp() const;
		double bhp() const;

		HRStat &operator +=(const HRStat &s);

		ostream &print(ostream &os, const String &hit, const String &miss, const String &pfx) const;

	protected:
		TmSzStat theHits;
		TmSzStat theMisses;
};

inline OLog &operator <<(OLog &ol, const HRStat &s) { return s.store(ol); }
inline ILog &operator >>(ILog &il, HRStat &s) { return s.load(il); }


/* inlined methods */

inline
void HRStat::record(const Time &tm, Size sz, bool hit) {
	hit ? theHits.record(tm, sz) : theMisses.record(tm, sz);
}

#endif
