
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_RPTMSTAT_H
#define POLYGRAPH__RUNTIME_RPTMSTAT_H

#include "xstd/AlarmClock.h"
#include "base/AggrStat.h"
#include "runtime/BcastRcver.h"

class RptmstatSym;
class StatPhase;

// response time "thermostat"
class Rptmstat: public AlarmUser, public BcastRcver {
	public:
		static bool IsEnabled; // XXX: to disable on server-side

	public:
		Rptmstat();

		void configure(const RptmstatSym &cfg);

		void start(StatPhase *aPhase);
		void stop(StatPhase *aPhase);

		virtual void wakeUp(const Alarm &alarm);
		virtual void noteXactEvent(BcastChannel *, const Xaction *);

	protected:
		void changeLoad(double delta);
		void nextSample();
		void checkpoint();

	protected:
		Time theSampleDur;   // sampling interval
		Time theRptmMin;     // rptm goals
		Time theRptmMax;
		double theLoadDelta; // reactivity

		StatPhase *thePhase;
		AggrStat theRptm;    // current measurements
};

#endif
