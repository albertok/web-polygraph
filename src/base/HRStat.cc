
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "base/HRStat.h"

#include "xstd/gadgets.h"


HRStat::HRStat() {
	reset();
}

void HRStat::reset() {
	theHits.reset();
	theMisses.reset();
}

bool HRStat::active() const {
	return theHits.active() || theMisses.active();
}

bool HRStat::sane() const {
	return theHits.sane() && theMisses.sane();
}

TmSzStat HRStat::xacts() const { 
	TmSzStat res = theHits;
	res += theMisses;
	return res;
}

OLog &HRStat::store(OLog &log) const {
	return log << theHits << theMisses;
}

ILog &HRStat::load(ILog &log) {
	return log >> theHits >> theMisses;
}

double HRStat::dhr() const {
	return Ratio(hits().count(), xacts().count());
}

double HRStat::bhr() const {
	return Ratio(hits().size().sum(), xacts().size().sum());
}

double HRStat::dhp() const {
	return Percent(hits().count(), xacts().count());
}

double HRStat::bhp() const {
	return Percent(hits().size().sum(), xacts().size().sum());
}

HRStat &HRStat::operator +=(const HRStat &hr) {
	theHits += hr.theHits;
	theMisses += hr.theMisses;
	return *this;
}

ostream &HRStat::print(ostream &os, const String &hit, const String &miss, const String &pfx) const {
	os << pfx << hit << ".ratio.obj: \t " << dhp() << endl;
	os << pfx << hit << ".ratio.byte:\t " << bhp() << endl;
	theHits.print(os, pfx + hit + ".");
	theMisses.print(os, pfx + miss + ".");
	return os;
}
