
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_TMSZSTAT_H
#define POLYGRAPH__BASE_TMSZSTAT_H

#include "base/AggrStat.h"

class OLog;
class ILog;

// manages (response time, size, count) related statistics
class TmSzStat {
	public:
		TmSzStat();
		TmSzStat(const AggrStat &aTm, const AggrStat &aSz);

		void reset();

		bool active() const;

		OLog &store(OLog &log) const;
		ILog &load(ILog &);
		bool sane() const { return time().sane() && size().sane(); }
		bool known() const { return time().known() || size().known(); }

		Counter count() const { return theTm.count() ? theTm.count() : theSz.count(); }
		const AggrStat &time() const { return theTm; }
		const AggrStat &size() const { return theSz; }

		// it is better to call this with valid tm and sz
		inline void record(const Time &tm, Size sz);
		
		void time(const AggrStat &aTm) { theTm = aTm; }
		void size(const AggrStat &aSz) { theSz = aSz; }
		TmSzStat &operator +=(const TmSzStat &s);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		AggrStat theTm;
		AggrStat theSz;
};

inline OLog &operator <<(OLog &ol, const TmSzStat &s) { return s.store(ol); }
inline ILog &operator >>(ILog &il, TmSzStat &s) { return s.load(il); }

inline
TmSzStat operator +(const TmSzStat &s1, const TmSzStat &s2) {
	TmSzStat res(s1);
	return res += s2;
}


/* inlined methods */

inline
void TmSzStat::record(const Time &tm, Size sz) {
	if (tm >= 0)
		theTm.record(tm.msec());
	if (sz >= 0)
		theSz.record(sz);
}

#endif
