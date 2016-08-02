
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>
#include "xstd/h/math.h"
#include "xstd/h/iomanip.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "base/Histogram.h"
#include "xstd/gadgets.h"


Histogram::Histogram(): theValMin(-1), theValMax(-1), theBinMax(-1) {
}

Histogram::Histogram(Val aMin, Val aMax, int binCount) {
	limits(aMin, aMax, binCount);
}

Histogram::Histogram(const Histogram &h) {
	add(h); // calls limits() to initialize our members first
}

void Histogram::limits(Val aMin, Val aMax, int binCount) {
	theBins.resize(binCount+2);
	theValMin = aMin;
	theValMax = aMax;
	theBinMax = binCount+1;
}

void Histogram::reset() {
	theStats.reset();
	theBins.clear();
	// do not reset 'limits'
}

void Histogram::record(Val v) {
	theStats.record(v);
	v -= theValMin;
	const int idx = v >= 0 ? Min(val2Bin(v)+1, theBinMax) : 0;
	theBins[idx]++;
}

void Histogram::add(const Histogram &h) {
	// brand-new histogram can adjust its limits to match h
	if (!theStats.count())
		limits(h.theValMin, h.theValMax, h.theBinMax - 1);

	// histograms must have identical structure
	Assert(theValMin == h.theValMin);
	Assert(theValMax == h.theValMax);
	Assert(theBinMax == h.theBinMax);
	Assert(theBins.count() == h.theBins.count());

	theStats += h.theStats;

	for (int i = 0; i < theBins.count(); ++i)
		theBins[i] += h.theBins[i];
}

Histogram &Histogram::operator +=(const Histogram &h) {
	add(h); 
	return *this; 
}

OLog &Histogram::store(OLog &log) const {
	log << theValMin << theValMax << theBinMax << theStats;

	// see if it is more efficient to log busy bins only
	int busyCount = 0;
	if (theStats.count()) {
		for (int i = 0; i < theBins.count(); ++i) {
			if (theBins[i])
				busyCount++;
		}
	}

	// save space if we save at least 20%
	const bool saveSpace = Percent(2*busyCount, theBins.count()) <= 80;
	log << saveSpace;

	if (saveSpace) {
		log << theBins.count();
		log << busyCount;
		// store <pos,bin> pairs
		for (int i = 0; i < theBins.count(); ++i) {
			if (theBins[i])
				log << i << theBins[i];
		}
	} else {
		log << theBins;
	}

	return log;
}

ILog &Histogram::load(ILog &log) {
	int vmin, vmax, bmax;
	log >> vmin >> vmax >> bmax >> theStats;
	Assert(theValMin == vmin);
	Assert(theValMax == vmax);
	Assert(theBinMax == bmax);

	if (log.getb()) {
		const int binCount = log.geti();
		const int busyCount = log.geti();
		Assert(binCount == theBins.count());

		// read <pos,bin> pairs
		for (int i = 0; i < busyCount; ++i) {
			const int pos = log.geti();
			Assert(0 <= pos && pos < theBins.count());
			log >> theBins[pos];
		}
	} else {
		log >> theBins;
	}

	return log;
}

void Histogram::report(double step, ostream &os) const {
	const Counter totCount = stats().count();
	Array<HistogramBin> percs;

	Percentiles(*this, percs, step);
	if (!percs.count())
		return;

	const int maxNum = percs.last().sup;
	const int numLen = maxNum > 1 ? (int)log10(maxNum-1.) : 1;
	const int vw = 1 + Max(numLen, 4);

	// header
	os
		<< "# bin"
		<< ' ' << setw(vw) << "min"
		<< ' ' << setw(vw) << "max"
		<< "   count     %   acc% "
		<< endl;

	int lastIdx = -1;
	for (int i = 0; i < percs.count(); ++i) {
		const HistogramBin &b = percs[i];
		if (b.idx != lastIdx) {
			os
				<< setw(5) << b.idx
				<< ' ' << setw(vw) << b.min
				<< ' ' << setw(vw) << b.sup-1
				<< ' ' << setw(7) << b.count
				<< ' ' << setw(5) << Percent(b.count, totCount)
				<< ' ' << setw(6) << Percent(b.accCount, totCount)
				<< endl;
			lastIdx = b.idx;
		}
	}
}

ostream &Histogram::print(ostream &os, const String &pfx) const {
	theStats.print(os, pfx);
	os << pfx << "hist:" << endl;
	report(0.01, os);
	os << endl;
	return os;
}

/* HistogramBin */

HistogramBin &HistogramBin::operator +=(const HistogramBin &b) {
	if (b.count) {
		if (count) {
			count += b.count;
			accCount = b.accCount;
			sup = b.sup;
			// min and idx stay the same
		} else {
			*this = b;
		}
	}
	return *this;
}

/* HistogramConstIter */

HistogramConstIter::HistogramConstIter(const Histogram &aHist):
	theHist(aHist) {
	theBin.reset();
	sync();
}

HistogramConstIter &HistogramConstIter::operator ++() {
	++theBin.idx;
	sync();
	return *this;
}

// must be called after reset or ++ (and nothing else)
void HistogramConstIter::sync() {
	Assert(theBin.idx >= 0);

	if (theBin.idx == 0) {
		theBin.min = -1; // what else can we put here?
		theBin.sup = theHist.theValMin;
	} else
	if (theBin.idx < theHist.theBinMax) {
		theBin.min = theBin.sup;
		theBin.sup = theHist.extract(theBin.idx+1);
	} else
	if (theBin.idx == theHist.theBinMax) {
		theBin.min = theBin.sup;
		theBin.sup = theHist.theValMax < INT_MAX ? theHist.theValMax+1 : INT_MAX;
	} else
		return;

	theBin.count = theHist.count(theBin.idx);
	theBin.accCount += theBin.count;
}



void Percentiles(const Histogram &hist, Array<HistogramBin> &percs, double pStep) {
	Assert(!percs.count());
	Assert(pStep > 0);
	int stepId = 1;

	const Counter totCount = hist.stats().count();
	HistogramBin accBin;
	int accCount = 0;

	for (HistogramConstIter i(hist); i; ++i) {
		const HistogramBin &b = *i;
		if (b.count) {
			accBin += b;
			accCount += b.count;
			if (accCount/(double)totCount >= stepId*pStep || accCount >= totCount) {
				do {
					percs.append(accBin);
					stepId++;
				} while (accCount/(double)totCount >= stepId*pStep);
				accBin.reset();
			}
		}
	}
}

