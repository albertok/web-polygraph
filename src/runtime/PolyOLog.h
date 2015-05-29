
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_POLYOLOG_H
#define POLYGRAPH__RUNTIME_POLYOLOG_H

#include "xstd/AlarmClock.h"
#include "base/OLog.h"

// records current time and progress info when time changes
// flushes the log from time to time
// writes magic markers
class PolyOLog: public OLog, public AlarmUser {
	public:
		PolyOLog();

		void period(Time aPeriod);

		virtual void begEntry(int tag);
		virtual void wakeUp(const Alarm &alarm);

	protected:
		virtual void putHeader();
		virtual void putTrailer();

	protected:
		Time thePeriod;
		Time theModTime;
};

extern PolyOLog TheOLog;
extern PolyOLog *TheSmplOLog;

#endif
