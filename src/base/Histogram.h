
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_HISTOGRAM_H
#define POLYGRAPH__BASE_HISTOGRAM_H

#include "xstd/Array.h"
#include "base/AggrStat.h"
#include "base/ILog.h"
#include "base/OLog.h"
#include "base/LogObj.h"

class HistogramConstIter;

// general purpose histogram abstraction
// value range: [min, max)
class Histogram: public LogObj {
	friend class HistogramConstIter;

	public:
		typedef int Val; // change to double if needed

	public:
		Histogram();
		Histogram(Val aMin, Val aMax, int binCount);
		Histogram(const Histogram &h);
		virtual ~Histogram() {}

		void limits(Val aMin, Val aMax, int binCount);
		virtual void reset();

		void record(Val v);

		virtual OLog &store(OLog &log) const;
		virtual ILog &load(ILog &);

		virtual void add(const Histogram &h);
		Histogram &operator +=(const Histogram &h);

		bool sane() const { return stats().sane(); }
		bool known() const { return stats().known(); }
		const AggrStat &stats() const { return theStats; }

		// extreme bins' counters
		bool extreme(int bin) const { return !bin || bin == theBinMax; }
		Counter underCount() const { return theBins[0]; }
		Counter overCount() const { return theBins[theBinMax]; }

		ostream &print(ostream &os, const String &pfx) const;
		void report(double step, ostream &os) const;

	protected:
		virtual const char *type() const = 0; // val2bin type
		virtual int val2Bin(Val v) const = 0; // value is already offset
		virtual Val bin2Val(int b) const = 0; // does not offset value

		Val extract(int b) const { return bin2Val(b-1) + theValMin; }
		Counter count(const int b) const { return theBins[b]; }

	protected:
		Array<Counter> theBins; // counters are stored here
		AggrStat theStats;
		Val theValMin;
		Val theValMax;
		int theBinMax;
};


// a virtual representation of a histogram "bin"
class HistogramBin {
	public:
		HistogramBin() { reset(); }
		void reset() { count = accCount = idx = 0; min = sup = -1; }

		HistogramBin &operator +=(const HistogramBin &b);

	public:
		Histogram::Val min;
		Histogram::Val sup; // max+1
		int idx;
		Counter count;
		Counter accCount; // cumulative count
};

// constant iterator for a histogram
class HistogramConstIter {
	public:
		HistogramConstIter(const Histogram &aHist);

		operator void*() const { return theBin.idx <= theHist.theBinMax ? (void*)-1 : 0; }
		const HistogramBin &operator *() const { return theBin; }
		const HistogramBin *operator ->() const { return &theBin; }
		HistogramConstIter &operator ++();

	private:
		void sync();

	protected:
		const Histogram &theHist;
		HistogramBin theBin; // current bin;
};


// builds percentiles array with a given step (1% default)
extern void Percentiles(const Histogram &, Array<HistogramBin> &percs, double step = 0.01);



#endif
