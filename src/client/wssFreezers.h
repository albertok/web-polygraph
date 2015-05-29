
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_WSSFREEZERS_H
#define POLYGRAPH__CLIENT_WSSFREEZERS_H

#include "xstd/AlarmClock.h"
#include "client/WssFreezer.h"

// waits till the specified time and triggers ieWssFreeze event
class TimeWssFreezer: public WssFreezer, public AlarmUser {
	public:
		TimeWssFreezer(Time aDelay);

		virtual void start();
		virtual void wakeUp(const Alarm &alarm);
		virtual void noteInfoEvent(BcastChannel *ch, InfoEvent ev);

	protected:
		virtual void printGoal(ostream &os) const;

	protected:
		Time theStartTime;
		Time theDelay;
};

// waits until the specified fill count and triggers ieWssFreeze event
class FillWssFreezer: public WssFreezer {
	public:
		FillWssFreezer(int aFillCount);

		virtual void noteXactEvent(BcastChannel *, const Xaction *);
		virtual void noteInfoEvent(BcastChannel *ch, InfoEvent ev);

	protected:
		virtual void printGoal(ostream &os) const;

	protected:
		int theFillCount;
};

#endif
