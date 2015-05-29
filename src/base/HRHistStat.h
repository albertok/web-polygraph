
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_HRHISTSTAT_H
#define POLYGRAPH__BASE_HRHISTSTAT_H

#include "base/TmSzHistStat.h"


// response time and histograms for "hits" and "misses"
// "hit" and "miss" classes must be disjoint
class HRHistStat {
	public:
		HRHistStat();

		void reset();

		inline void record(const Time &tm, Size sz, bool hit);

		const TmSzHistStat &hits() const { return theHits; }
		const TmSzHistStat &misses() const { return theMisses; }
		TmSzStat aggr() const { return theHits.aggr() + theMisses.aggr(); }

		HRHistStat &operator +=(const HRHistStat &s);

		OLog &store(OLog &log) const;
		ILog &load(ILog &);

		ostream &print(ostream &os, const String &hit, const String &miss, const String &pfx) const;

	protected:
		TmSzHistStat theHits;
		TmSzHistStat theMisses;
};

inline OLog &operator <<(OLog &ol, const HRHistStat &s) { return s.store(ol); }
inline ILog &operator >>(ILog &il, HRHistStat &s) { return s.load(il); }


/* inlined methods */

inline
void HRHistStat::record(const Time &tm, Size sz, bool hit) {
	hit ? theHits.record(tm, sz) : theMisses.record(tm, sz);
}

#endif
