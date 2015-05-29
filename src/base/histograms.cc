
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "base/ILog.h"
#include "base/OLog.h"

#include "base/histograms.h"



/* Log2Hist */

/*
 *   pow2       min :        max /precision =#entries (accum size)
 *     8          0 :        511 /        1 = 512  (  512 or   2K)
 *     9        512 :       1023 /        2 = 256  (  768 or   3K)
 *    10       1024 :       2047 /        4 = 256  ( 1024 or   4K)
 *    11       2048 :       4095 /        8 = 256  ( 1280 or   5K)
 *    12       4096 :       8191 /       16 = 256  ( 1536 or   6K)
 *    13       8192 :      16383 /       32 = 256  ( 1792 or   7K)
 *    14      16384 :      32767 /       64 = 256  ( 2048 or   8K)
 *    15      32768 :      65535 /      128 = 256  ( 2304 or   9K)
 *    16      65536 :     131071 /      256 = 256  ( 2560 or  10K)
 *    17     131072 :     262143 /      512 = 256  ( 2816 or  11K)
 *    18     262144 :     524287 /     1024 = 256  ( 3072 or  12K)
 *    19     524288 :    1048575 /     2048 = 256  ( 3328 or  13K)
 *    20    1048576 :    2097151 /     4096 = 256  ( 3584 or  14K)
 *    21    2097152 :    4194303 /     8192 = 256  ( 3840 or  15K)
 *    22    4194304 :    8388607 /    16384 = 256  ( 4096 or  16K)
 *    23    8388608 :   16777215 /    32768 = 256  ( 4352 or  17K)
 *    24   16777216 :   33554431 /    65536 = 256  ( 4608 or  18K)
 *    25   33554432 :   67108863 /   131072 = 256  ( 4864 or  19K)
 *    26   67108864 :  134217727 /   262144 = 256  ( 5120 or  20K)
 *    27  134217728 :  268435455 /   524288 = 256  ( 5376 or  21K)
 *    28  268435456 :  536870911 /  1048576 = 256  ( 5632 or  22K)
 *    29  536870912 : 1073741823 /  2097152 = 256  ( 5888 or  23K)
 *    30 1073741824 : 2147483647 /  4194304 = 256  ( 6144 or  24K)
 */

// note: val2Bin has static binding here
Log2Hist::Log2Hist(): Histogram(0, INT_MAX, val2Bin(INT_MAX)) {
}

// note: val2Bin has static binding here
Log2Hist::Log2Hist(Val aMin, Val aMax): 
	Histogram(aMin, aMax, Log2Hist::val2Bin(aMax-aMin)) {
}

int Log2Hist::val2Bin(Val v) const {
	Assert(v >= 0);
	const int vo = v; 
	if (v >>= 9) {
		int p = 1;
		while (v >>= 1) p++; // count powers of 2; (super-bin id)
		const int min_val = 1 << (8+p);
		const int sbin_offset = (vo - min_val) >> p;
		const int sbin = (p+1) << 8;
		return sbin + sbin_offset;
	} else
		return vo;
}

Log2Hist::Val Log2Hist::bin2Val(int bin) const {
	const int bino = bin;
	if (bin >= 512) {
		const int bin_offset = bin % 256;
		const int sbin = bin - bin_offset;
		const int p = sbin/256 - 1;
		const int val_offset = bin_offset << p;
		const int min_val = 1 << (8+p);
		return min_val + val_offset;
	} else
		return bino;
}


/* TimeHist */

TimeHist::TimeHist(): theTmMax(Time::Msec(theValMax)) {
	Assert(theTmMax > 0);
}


/* TimeVsSmthHist */

static inline
OLog &operator <<(OLog &log, const TimeHist *h) {
	Assert(h);
	return log << *h;
}

static inline
ILog &operator >>(ILog &log, TimeHist *h) {
	Assert(h);
	return log >> *h;
}


TimeVsSmthHist::TimeVsSmthHist(int aMaxSmth): theMaxSmth(aMaxSmth) {
}

TimeVsSmthHist::~TimeVsSmthHist() {
	while (theHists.count()) delete theHists.pop();
}

void TimeVsSmthHist::resize(int maxGrp) {
	Assert(!theHists.count());
	Assert(maxGrp >= 0);
	const int count = 1 + maxGrp;
	theHists.stretch(count);
	for (int i = 0; i < count; ++i)
		theHists.append(new TimeHist);
}

OLog &TimeVsSmthHist::store(OLog &log) const {
	return log << theMaxSmth << theHists;
}


ILog &TimeVsSmthHist::load(ILog &log) {
	const int ms = log.geti();
	Must(ms <= theMaxSmth);
	theMaxSmth = ms;
	return log >> theHists;
}

// log2 step except a special case of 100 bytes
// range must be checked in the caller
int TimeVsSmthHist::logSmth(int smth_off) const {
	Assert(smth_off > 0);
	int n = 0;
	while (smth_off >>= 1) n++; // log2
	return n;
}

int TimeVsSmthHist::expSmth(int grp_off) const {
	Assert(grp_off >= 0);
	int smth_off = 1;
	while (grp_off--) smth_off <<= 1; // exp2
	return smth_off;
}

void TimeVsSmthHist::add(const TimeVsSmthHist &h) {
	// histograms must match
	Assert(theMaxSmth == h.theMaxSmth);
	Assert(theHists.count() == h.theHists.count());

	for (int i = 0; i < theHists.count(); ++i)
		theHists[i]->add(*h.theHists[i]);
}

void TimeVsSmthHist::reset() {
	for (int i = 0; i < theHists.count(); ++i)
		theHists[i]->reset();
}

void TimeVsSmthHist::printVsLine(const Histogram &h, int pStep, ostream &os) const {
	// print count and mean
	os << ' ' << setw(7) << h.stats().count()
		<< ' ' << setw(9) << h.stats().mean();

	// get and print percentiles
	Array<HistogramBin> percs;
	Percentiles(h, percs);
	for (int i = pStep; i <= 100; i += pStep) {
		if (i <= percs.count()) // do we ever have less?
			os << ' ' << setw(3) << percs[i-1].sup-1;
		else
			os << ' ' << setw(3) << '?';
	}
}

ostream &TimeVsSmthHist::print(ostream &os, const String &type, int min, const String &pfx) const {
	const int pStep = 5; // 5%;

	// header
	os << pfx << "hist:" << endl 
		<< "#   " << setw(4) << type << "              response_time" << endl
		<< "# min    max   count      mean";
	for (int i = pStep; i <= 100; i += pStep)
		os << ' ' << setw(2) << i << '%';
	os << endl;

	// for each size group, print mean and percentiles
	const int lastGrp = lastGrpId();
	for (int g = 0; g <= lastGrp; ++g) {
		const Histogram &h = groupHist(g);
		if (!h.stats().count())
			continue;

		int max = groupMax(g);

		os << setw(5) << min << ' ' << setw(6) << max;
		printVsLine(h, pStep, os);
		os << endl;

		min = max + 1;
	}
	os << endl;
	return os;
}

/* TimeVsSizeHist */

TimeVsSizeHist::TimeVsSizeHist(): TimeVsSmthHist(Size::KB(128)-Size(1)) {
	resize(TimeVsSizeHist::groupId(theMaxSmth));
}

// special case of 0-100 and 101-1023 ranges
// range must be checked in the caller
int TimeVsSizeHist::groupId(int sz) const {
	if (sz < 100)
		return 0;
	if (sz < 1024)
		return 1;
	return 2 + logSmth(sz/1024);
}

int TimeVsSizeHist::groupMax(int grp) const {
	Assert(grp >= 0);
	if (grp == 0)
		return 100 - 1;
	if (grp == 1)
		return 1024 - 1;

	return 1024*expSmth(grp-2 + 1) - 1;
}

ostream &TimeVsSizeHist::print(ostream &os, const String &pfx) const {
	return TimeVsSmthHist::print(os, "size", 0, pfx);
}


/* TimeVsUseHist */

TimeVsUseHist::TimeVsUseHist(): TimeVsSmthHist(128-1) {
	resize(TimeVsUseHist::groupId(theMaxSmth));
}

// special cases for 1,2,3,4 uses
// range must be checked in the caller
int TimeVsUseHist::groupId(int useCnt) const {
	if (useCnt <= 4)
		return useCnt-1;

	return 4 + logSmth(useCnt - 4);
}

int TimeVsUseHist::groupMax(int grp) const {
	Assert(grp >= 0);
	if (grp < 4)
		return grp+1;

	return 4+expSmth(grp-4 + 1) - 1;
}

ostream &TimeVsUseHist::print(ostream &os, const String &pfx) const {
	return TimeVsSmthHist::print(os, "use", 1, pfx);
}
