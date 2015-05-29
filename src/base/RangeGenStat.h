
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_RANGEGENSTAT_H
#define POLYGRAPH__BASE_RANGEGENSTAT_H

#include "base/AggrStat.h"

class OLog;
class ILog;

// range generation statistics

class RangeGenStat {
	public:
		RangeGenStat();

		void reset();

		OLog &store(OLog &log) const;
		ILog &load(ILog &log);

		inline void recordOneSize(const Size &sz);
		inline void recordTotalSize(const Size &sz);
		inline void recordOneOffOver();
		inline void recordAbsRelSwap();
		inline void recordTotalSizeOver();

		RangeGenStat &operator +=(const RangeGenStat &s);

		ostream &print(ostream &os, const String &pfx) const;

	protected:

		AggrStat theOneSize;
		AggrStat theTotalSize;
		int theAbsRelSwapCount;
		int theOneOffOverCount;
		int theTotalSizeOverCount;
};

/* inlined methods */

inline
void RangeGenStat::recordOneSize(const Size &sz) {
	if (sz >= 0)
		theOneSize.record(sz);
}

inline
void RangeGenStat::recordTotalSize(const Size &sz) {
	if (sz >= 0)
		theTotalSize.record(sz);
}

inline
void RangeGenStat::recordOneOffOver() {
	++theOneOffOverCount;
}

inline
void RangeGenStat::recordAbsRelSwap() {
	++theAbsRelSwapCount;
}

inline
void RangeGenStat::recordTotalSizeOver() {
	++theTotalSizeOverCount;
}

inline
ostream &operator <<(ostream &os, const RangeGenStat &s) { return s.print(os, ""); }

inline
OLog &operator <<(OLog &log, const RangeGenStat &s) { s.store(log); return log; }

inline
ILog &operator >>(ILog &log, RangeGenStat &s) { s.load(log); return log; }

#endif
