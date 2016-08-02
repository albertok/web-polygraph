
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_COMPOUNDXACTSTAT_H
#define POLYGRAPH__BASE_COMPOUNDXACTSTAT_H

#include "base/histograms.h"
#include "base/LogObj.h"

// stats for CompoundXactInfo objects
class CompoundXactStat: public LogObj {
	public:
		CompoundXactStat();

		void reset();

		virtual OLog &store(OLog &log) const;
		virtual ILog &load(ILog &);

		CompoundXactStat &operator +=(const CompoundXactStat &s);

		bool sane() const;

		void recordCompound(const Time &rptm, const Size &reqSize, const Size &repSize, const int exchanges);

		ostream &print(ostream &os, const String &pfx) const;

		TimeHist duration; // transaction duration
		SizeHist reqSize; // transaction requests size
		SizeHist repSize; // transaction replies size
		LineHist exchanges; // number of exchanges in transaction
};

inline OLog &operator <<(OLog &ol, const CompoundXactStat &s) { return s.store(ol); }
inline ILog &operator >>(ILog &il, CompoundXactStat &s) { return s.load(il); }

#endif
