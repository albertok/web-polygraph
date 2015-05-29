
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "probe/ProbeLinkStat.h"

ProbeLinkStat::ProbeLinkStat(): theConnCnt(0), theErrorCnt(0) {
}

void ProbeLinkStat::recordConn() {
	theConnCnt++;
	syncDur();
}

void ProbeLinkStat::recordError(const Error &) {
	theErrorCnt++;
	syncDur();
}

void ProbeLinkStat::recordRead(Size sz) {
	theSockRdSzH.record(sz);
	syncDur();
}

void ProbeLinkStat::recordWrite(Size sz) {
	theSockWrSzH.record(sz);
	syncDur();
}

void ProbeLinkStat::syncWith(const ProbeLinkStat &s) {
	if (theSockRdSzH.stats().count() > s.theSockRdSzH.stats().count()) {
		theSockRdSzH.reset();
		theSockRdSzH += s.theSockRdSzH;
	}
	if (theSockWrSzH.stats().count() > s.theSockWrSzH.stats().count()) {
		theSockWrSzH.reset();
		theSockWrSzH += s.theSockWrSzH;
	}
}

ProbeLinkStat &ProbeLinkStat::operator +=(const ProbeLinkStat &s) {
	theSockRdSzH += s.theSockRdSzH;
	theSockWrSzH += s.theSockWrSzH;
	theConnCnt += s.theConnCnt;
	theErrorCnt += s.theErrorCnt;

	// XXX: pessimistic assumptions
	if (theStart < 0 || (s.theStart >= 0 && s.theStart < theStart))
		theStart = s.theStart;
	if (theEnd < 0 || (s.theEnd >= 0 && s.theEnd > theEnd))
		theEnd = s.theEnd;

	return *this;
}

void ProbeLinkStat::syncDur() {
	if (theStart < 0)
		theStart = TheClock;
	theEnd = TheClock;
}

void ProbeLinkStat::store(OLog &log) const {
	log << theSockRdSzH << theSockWrSzH
		<< theConnCnt << theErrorCnt
		<< theStart << theEnd;
}

void ProbeLinkStat::load(ILog &log) {
	log >> theSockRdSzH >> theSockWrSzH
		>> theConnCnt >> theErrorCnt
		>> theStart >> theEnd;
}

ostream &ProbeLinkStat::print(ostream &os, const String &pfx) const {
	theSockRdSzH.print(os, pfx + "so_read.size.");
	theSockWrSzH.print(os, pfx + "so_write.size.");
	os << pfx << ".conn.count: " << theConnCnt << endl;
	os << pfx << ".errors.count: " << theErrorCnt << endl;
	os << pfx << ".duration: " << duration().secd() << endl;
	return os;
}

