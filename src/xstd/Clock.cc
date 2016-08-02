
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/Array.h"
#include "xstd/Clock.h"

static void RegisterClock(Clock *clock);
static void UnregisterClock(Clock *clock);

Clock &TheClock = *new Clock;
static Array<Clock*> *TheClocks = 0;

/* Clock */

Time Clock::TheStartTime;

Clock::Clock() {
	RegisterClock(this);
}

Clock::~Clock() {
	UnregisterClock(this);
}

void Clock::update(Time aCurTime) { 
	// gettimeofday(2) on FreeBSD-3.3 is buggy
	// make sure clocks never go backwards
	if (aCurTime > theCurTime && aCurTime.tv_usec >= 0)
		theCurTime = aCurTime;
}

void Clock::Update(bool advanceAll) {
	TheClock.update(Time::Now());

	if (TheStartTime < 0)
		TheStartTime = TheClock;

	if (advanceAll) {
		for (int i = TheClocks->count()-1; i >=0 ; --i) {
			Clock *&c = TheClocks->item(i);
			if (c && c != &TheClock)
				TheClocks->item(i)->update(TheClock);
		}
	}
}


static
void RegisterClock(Clock *clock) {
	Assert(clock);
	if (!TheClocks)
		TheClocks = new Array<Clock*>;
	TheClocks->append(clock);
}

static
void UnregisterClock(Clock *clock) {
	for (int i = TheClocks->count()-1; i >=0 ; --i) {
		if ((*TheClocks)[i] == clock) {
			(*TheClocks)[i] = (*TheClocks)[TheClocks->count()-1];
			TheClocks->pop(1);
			return;
		}
	}
	Assert(false); // clock not found
}
