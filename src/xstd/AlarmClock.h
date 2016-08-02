
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_ALARMCLOCK_H
#define POLYGRAPH__XSTD_ALARMCLOCK_H

#include "xstd/Heap.h"
#include "xstd/Clock.h"

class AlarmUser;

// one time alarm
class Alarm {
	public:
		Alarm(): theSleeper(0) {}
		Alarm(Time aTime, AlarmUser *aSleeper): theTime(aTime), theSleeper(aSleeper) {}

		operator void*() const { return theSleeper ? (void*)-1 : 0; }
		const Time &time() const { return theTime; }

		const AlarmUser *sleeper() const { return theSleeper; }

		void set(Time aTime, AlarmUser *aSleeper);
		void ring();
		void snooze();
		void clear() { theSleeper = 0; }

		// these are used by AlarmQueue to order the alarms
		bool operator <(const Alarm &a) const { return theTime < a.theTime; }
		bool operator >(const Alarm &a) const { return theTime > a.theTime; }

	protected:
		Time theTime;
		AlarmUser *theSleeper;
};

// somebody who can wait for an alarm
// alarm count is used to check if pending alarms must be cleared 
// before object destruction
class AlarmUser {
	public:
		AlarmUser();
		virtual ~AlarmUser();

		bool alarmWaiting() const { return thePendAlarmCnt != 0; }

		virtual void wakeUp(const Alarm &alarm) = 0;  // but has a body!
		virtual void snooze(const Alarm &alarm); // the alarm got cancelled

	protected:
		Alarm sleepTill(const Time &time);
		Alarm sleepFor(const Time &delay);
		void cancelAlarms();

	protected:
		int thePendAlarmCnt;
};

// a sequence of ordered Alarms
typedef Heap<Alarm> AlarmQueue;


// alarm clock that can handle unlimited number of alarms
class AlarmClock: public Clock {
	public:
		AlarmClock();
		~AlarmClock() {}

		bool on();
		Time timeLeft() const;

		virtual void update(Time curTime); // rings if needed

		void setAlarm(const Alarm &alarm);
		void cancelAll(const AlarmUser *sleeper);

	protected:
		void ring(); // wakes up ready sleepers

	protected:
		AlarmQueue theQueue;
};

extern AlarmClock &TheAlarmClock;

#endif
