
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/math.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "base/AggrStat.h"
#include "xstd/gadgets.h"


AggrStat::AggrStat() {
	reset();
}

void AggrStat::reset() {
	theCount = 0;
	theSum = theSqSum = 0;
	theMax = theMin = -1;
}

void AggrStat::record(Val v) {

	if (theCount++) {
		theSum += v;
		theSqSum += v*(double)v;

		if (theMax < v)
			theMax = v;
		else
		if (theMin > v)
			theMin = v;
	} else {
		theSum = theMin = theMax = v;
		theSqSum = v*(double)v;
	}
}

AggrStat &AggrStat::operator +=(const AggrStat &s) {
	if (s.theCount) {
		if (theCount) {
			theCount += s.theCount;
			theSum += s.theSum;
			theSqSum += s.theSqSum;
			theMax = Max(theMax, s.theMax);
			theMin = Min(theMin, s.theMin);
		} else {
			theCount = s.theCount;
			theSum = s.theSum;
			theSqSum = s.theSqSum;
			theMax = s.theMax;
			theMin = s.theMin;
		}
	}
	return *this;
}

double AggrStat::stdDev() const {
	if (theCount < 2)
		return -1;

	const double diff = theSqSum - theSum*theSum/theCount;
	if (diff < 0)
		return 0; // should not happen

	return sqrt(diff / (theCount-1));
}

double AggrStat::relDevp() const {
	return theCount > 1 ? 100*Ratio(stdDev(), mean()) : -1.0;
}

void AggrStat::store(OLog &log) const {
	log << theCount;
	if (theCount)
		log << theMin << theMax << theSum << theSqSum;
}

void AggrStat::load(ILog &log) {
	log >> theCount;
	if (theCount)
		log >> theMin >> theMax >> theSum >> theSqSum;
}

bool AggrStat::sane() const {
	return !theCount ||
		(theCount > 0 && theMin <= theMax && theSqSum >= 0);
}

ostream &AggrStat::print(ostream &os, const String &pfx) const {
	return os
		<< pfx << "count:  \t " << theCount << endl
		<< pfx << "mean:   \t " << mean() << endl
		<< pfx << "min:    \t " << theMin << endl
		<< pfx << "max:    \t " << theMax << endl
		<< pfx << "std_dev:\t " << stdDev() << endl
		<< pfx << "rel_dev:\t " << relDevp() << endl
		<< pfx << "sum:    \t " << theSum << endl
		;
}
