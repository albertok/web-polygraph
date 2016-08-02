
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "xstd/gadgets.h"
#include "base/ILog.h"
#include "base/OLog.h"
#include "base/ErrorRec.h"


ErrorRec::ErrorRec() {
	reset();
}

ErrorRec::ErrorRec(const ErrorRec &rec): Error(rec) {
	theCount = rec.theCount;
	theRepCount = rec.theRepCount;
	theRepTime = rec.theRepTime;
	theText = rec.theText;
	theNext = 0;
}

ErrorRec::ErrorRec(const Error &e): Error(e) {
	reset();
	theText = e.str();
}

ErrorRec::~ErrorRec() {
}

void ErrorRec::reset() {
	theCount = 0;
	theRepCount = 0;
	theRepTime = Time();
	theText = 0;
	theNext = 0;
}

bool ErrorRec::needReport() const {
	return 
		theRepCount <= theCount ||
		theRepTime <= TheClock;
}

void ErrorRec::noteReport(const Time &tout) {
	theRepCount = theCount > 0 ? 2 * theCount : 1;
	theRepTime = TheClock + tout;
}

void ErrorRec::store(OLog &ol) const {
	ol << theNo << theCount << theRepCount << theRepTime << theText;
}

void ErrorRec::load(ILog &il) {
	reset();
	il >> theNo >> theCount >> theRepCount >> theRepTime >> theText;
}

void ErrorRec::add(const ErrorRec &e) {
	Assert(theNo == e.theNo);
	theCount += e.theCount;
	theRepCount += theRepCount;
	theRepTime = Max(theRepTime, e.theRepTime);
}
