
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "base/Progress.h"
#include "runtime/PolyOLog.h"
#include "base/polyLogTags.h"

// assume no static-ctors will log
PolyOLog TheOLog; 
PolyOLog *TheSmplOLog = 0;


PolyOLog::PolyOLog() {
	// stretch to the maximum
	theDir.put(0, lgEnd);
}

void PolyOLog::period(Time aPeriod) {
	Assert(aPeriod > 0);
	Assert(theStream);
	thePeriod = aPeriod;
	sleepFor(thePeriod);
}

void PolyOLog::wakeUp(const Alarm &alarm) {
	const Size pageSize = 4096; // XXX: typical page size?
	AlarmUser::wakeUp(alarm);
	if (theStream) {
		// flush at most two pages at a time
		if (size() >= pageSize)
			flush(size() >= pageSize*2 ? pageSize*2 : pageSize);
		
		sleepTill(TheClock + thePeriod);
	}
	// do not set alarms if closed
}

void PolyOLog::putHeader() {
	puti(lgMagic1); puti(lgMagic2); puti(0); // magic
	OLog::putHeader();
}

void PolyOLog::putTrailer() {
	OLog::putTrailer();
	puti(0); puti(lgMagic2); puti(lgMagic1); // magic
}

void PolyOLog::begEntry(int tag) {

	if (theModTime != TheClock) {
		theModTime = TheClock;
		OLog::begEntry(lgProgress);
		TheProgress.store(*this);
		endEntry();
	}

	OLog::begEntry(tag);
}
