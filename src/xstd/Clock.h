
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_CLOCK_H
#define POLYGRAPH__XSTD_CLOCK_H

#include "xstd/Time.h"

// an object that can show current time and responds to time changes
// clocks are syncronized via Clock::Update() call

class Clock {
	public:
		static void Update(bool advanceAll = true);
		static Time TheStartTime;

	public:
		Clock();
		virtual ~Clock();

		Time time() const { return theCurTime; }
		operator Time() const { return theCurTime; }
		Time runTime() const { return theCurTime - TheStartTime; }
	
		Time now() { update(Time::Now()); return theCurTime; }
		virtual void update(Time aCurTime);

	protected:
		Time theCurTime;
};

// "Wall clock" or "official clock" (it is always advanced by UpdateClocks())
extern Clock &TheClock;

#endif
