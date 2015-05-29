
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_HISTOGRAMS_H
#define POLYGRAPH__BASE_HISTOGRAMS_H

#include "base/Histogram.h"
#include "xstd/gadgets.h"

class ILog;
class OLog;

// bunch of histograms


// linear histogram (currently only 1:1 mapping is supported, no scaling)
class LineHist: public Histogram {
	public:
		LineHist() {}
		LineHist(Val aMin, Val aMax): Histogram(aMin, aMax, aMax-aMin) {}

	protected:
		virtual const char *type() const { return "line"; }
		virtual int val2Bin(Val v) const { return v; }
		virtual Val bin2Val(int b) const { return b; }
};


// log2-based histogram
// fast (no fp math!) and takes about 25KB for [0-MAX_INT) range (default)
// suitable for time and size measurements
// see the definition for details
class Log2Hist: public Histogram {
	public:
		Log2Hist();
		Log2Hist(Val aMin, Val aMax);

	protected:
		virtual const char *type() const { return "log2"; }
		virtual int val2Bin(Val v) const;
		virtual Val bin2Val(int b) const;
};


// Time histogram (internal representation uses msec)
class TimeHist: public Log2Hist {
	public:
		TimeHist();

		void record(const Time &tm) { record(Min(tm, theTmMax).msec()); }

	protected:
		void record(Val v) { Histogram::record(v); }

	protected:
		Time theTmMax;
};

typedef Log2Hist SizeHist;

// common class for TimeVs* histograms
// though called *Hist, this is a set of histograms, actually
class TimeVsSmthHist: public LogObj {
	public:
		TimeVsSmthHist(int aMaxSmth);
		virtual ~TimeVsSmthHist();

		int lastGrpId() const { return groupId(theMaxSmth); }
		virtual int groupMax(int g) const = 0;
		const TimeHist &groupHist(int g) const { return *theHists[g]; }

		virtual OLog &store(OLog &log) const;
		virtual ILog &load(ILog &log);

		void add(const TimeVsSmthHist &h);

		void reset();

		ostream &print(ostream &os, const String &type, int min, const String &pfx) const;

	protected:
		virtual int groupId(int smth) const = 0;

		int logSmth(int smth) const;
		int expSmth(int grp) const;

		void resize(int maxGrp);

		void printVsLine(const Histogram &h, int pStep, ostream &os) const;

	protected:
		Array<TimeHist*> theHists;
		int theMaxSmth;
};


// ignores negative and huge sizes
class TimeVsSizeHist: public TimeVsSmthHist {
	public:
		TimeVsSizeHist();

		inline void record(const Time &tm, Size sz);

		virtual int groupMax(int g) const;

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual int groupId(int smth) const;
};

// scales use cnt using base-2 log
class TimeVsUseHist: public TimeVsSmthHist {
	public:
		TimeVsUseHist();

		inline void record(const Time &tm, int useCnt);

		virtual int groupMax(int g) const;

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual int groupId(int smth) const;
};


/* inlined methods */

inline
void TimeVsSizeHist::record(const Time &tm, Size sz) {
	if (0 <= sz && sz <= theMaxSmth)
		theHists[groupId(sz)]->record(tm);
}

inline
void TimeVsUseHist::record(const Time &tm, int useCnt) {
	if (0 < useCnt && useCnt <= theMaxSmth)
		theHists[groupId(useCnt)]->record(tm);
}

#endif
