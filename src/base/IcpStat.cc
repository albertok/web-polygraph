
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "base/ILog.h"
#include "base/OLog.h"
#include "base/IcpStat.h"


IcpStat::IcpStat() {
	reset();
}

void IcpStat::reset() {
	theStat.reset();
	theToutCnt = 0;
}

bool IcpStat::sane() const {
	return theStat.sane() && theToutCnt >= 0;
}

double IcpStat::reqRate(Time duration) const {
	return duration > 0 ?
		Ratio(theToutCnt + theStat.xacts().count(), duration.secd()) : -1;
}

double IcpStat::repRate(Time duration) const {
	return duration > 0 ?
		Ratio(theStat.xacts().count(), duration.secd()) : -1;
}

OLog &IcpStat::store(OLog &log) const {
	return log << theStat << theToutCnt;
}

ILog &IcpStat::load(ILog &log) {
	return log >> theStat >> theToutCnt;
}

IcpStat &IcpStat::operator +=(const IcpStat &ts) {
	theStat += ts.theStat;
	theToutCnt += ts.theToutCnt;
	return *this;
}

ostream &IcpStat::print(ostream &os, const String &pfx, Time duration) const {
	os << pfx << "req.rate:\t " << reqRate(duration) << endl;
	os << pfx << "rep.rate:\t " << repRate(duration) << endl;
	os << pfx << "timeout.count:\t " << theToutCnt << endl;

	theStat.xacts().print(os, pfx + "rep.");
	theStat.print(os, "hit", "miss", pfx);
	return os;
}
