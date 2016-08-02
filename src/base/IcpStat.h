
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_ICPSTAT_H
#define POLYGRAPH__BASE_ICPSTAT_H

#include "base/HRStat.h"

class IcpStat {
	public:
		IcpStat();

		void reset();

		bool sane() const;

		void record(const Time &tm, Size sz, bool hit) { theStat.record(tm, sz, hit); }
		void recordTimeout() { theToutCnt++; }

		IcpStat &operator +=(const IcpStat &ts);

		OLog &store(OLog &log) const;
		ILog &load(ILog &log);

		ostream &print(ostream &os, const String &pfx, Time duration) const;

	protected:
		double reqRate(Time duration) const;
		double repRate(Time duration) const;

	protected:
		HRStat theStat;
		Counter theToutCnt; // number of ICP timeouts
};

inline OLog &operator <<(OLog &ol, const IcpStat &s) { return s.store(ol); }
inline ILog &operator >>(ILog &il, IcpStat &s) { return s.load(il); }

#endif
