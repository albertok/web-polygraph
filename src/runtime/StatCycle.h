
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_STATCYCLE_H
#define POLYGRAPH__RUNTIME_STATCYCLE_H

#include "xstd/Array.h"
#include "runtime/StatIntvl.h"

class Notifier;
class StatCycleRec;

// StatIntvRec stats are reset and logged every [short] time interval

class StatCycle: public StatIntvl {
	public:
		StatCycle();
		virtual ~StatCycle();

		void period(Time aPeriod);
		void notifier(Notifier *aNotifier);

		void start();

		virtual void wakeUp(const Alarm &alarm);

	protected:
		virtual StatIntvlRec &getRec(int cat);
		virtual const StatIntvlRec &getRec(int cat) const;

		void nextIntvl(Time wakeUpTime);
		void restart();
		void report(const StatCycleRec &rec) const;

	protected:
		Array<StatCycleRec*> theRecs;
		Notifier *theNotifier;
		Time thePeriod;
};

#endif
