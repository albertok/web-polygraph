
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_AGGRSTAT_H
#define POLYGRAPH__BASE_AGGRSTAT_H

#include "xstd/h/iosfwd.h"
#include "xstd/h/stdint.h"
#include "base/OLog.h"
#include "base/ILog.h"


#ifdef min
#undef min
#endif
 
#ifdef max
#undef max
#endif

// simple aggregate statisitcs
class AggrStat {
	public:
		typedef int64_t Val;

	public:
		AggrStat();

		void reset();
		void store(OLog &log) const;
		void load(ILog &log);
		bool sane() const;

		void record(Val val);

		AggrStat &operator +=(const AggrStat &s);

		bool known() const { return count() > 0; }
		Counter count() const { return theCount; }
		Val min() const { return theMin; }
		Val max() const { return theMax; }
		double sum() const { return theSum; }
		double mean() const { return theCount ? theSum/theCount : -1; }
		double stdDev() const;
		double relDevp() const;

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		Counter theCount;
		Val theMax;
		Val theMin;
		double theSum;
		double theSqSum;
};

inline
ostream &operator <<(ostream &os, const AggrStat &s) { return s.print(os, ""); }

inline
AggrStat operator +(const AggrStat &s1, const AggrStat &s2) {
	AggrStat s(s1);
	return s += s2;
}

inline
OLog &operator <<(OLog &log, const AggrStat &s) { s.store(log); return log; }

inline
ILog &operator >>(ILog &log, AggrStat &s) { s.load(log); return log; }

#endif
