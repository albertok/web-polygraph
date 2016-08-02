
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_TMSZHISTSTAT_H
#define POLYGRAPH__BASE_TMSZHISTSTAT_H

#include "base/TmSzStat.h"
#include "base/histograms.h"

// manages (response time, size, count) related statistics
class TmSzHistStat {
	public:
		TmSzHistStat();

		void reset();

		OLog &store(OLog &log) const;
		ILog &load(ILog &);

		bool known() const { return time().known() || size().known(); }

		const TimeHist &time() const { return theTm; }
		const SizeHist &size() const { return theSz; }
		TmSzStat aggr() const { return TmSzStat(theTm.stats(), theSz.stats()); }

		// it is better to call this with valid tm and sz
		inline void record(const Time &tm, Size sz);

		TmSzHistStat &operator +=(const TmSzHistStat &s);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		TimeHist theTm;
		SizeHist theSz;
};

inline OLog &operator <<(OLog &ol, const TmSzHistStat &s) { return s.store(ol); }
inline ILog &operator >>(ILog &il, TmSzHistStat &s) { return s.load(il); }


/* inlined methods */

inline
void TmSzHistStat::record(const Time &tm, Size sz) {
	if (tm >= 0)
		theTm.record(tm);
	if (sz >= 0)
		theSz.record(sz);
}

#endif
