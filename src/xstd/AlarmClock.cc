
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/Assert.h"
#include "xstd/AlarmClock.h"
#include "xstd/gadgets.h"

AlarmClock &TheAlarmClock = *new AlarmClock;

/* Alarm */

void Alarm::ring() {
	Assert(theSleeper);
	theSleeper->wakeUp(*this);
	clear();
}

void Alarm::snooze() {
	Assert(theSleeper);
	theSleeper->snooze(*this);
	clear();
}

void Alarm::set(Time aTime, AlarmUser *aSleeper) {
	Assert(aSleeper);

	theSleeper = aSleeper;
	theTime = aTime;
}


/* AlarmUser */

AlarmUser::AlarmUser(): thePendAlarmCnt(0) {
}

AlarmUser::~AlarmUser() {
	cancelAlarms();
}

void AlarmUser::wakeUp(const Alarm &) {
	thePendAlarmCnt--;
	Assert(thePendAlarmCnt >= 0);
}

// ignore this alarm
void AlarmUser::snooze(const Alarm &) {
	thePendAlarmCnt--;
	Assert(thePendAlarmCnt >= 0);
}

Alarm AlarmUser::sleepTill(const Time &time) {
	const Alarm alarm(time, this);
	TheAlarmClock.setAlarm(alarm);
	thePendAlarmCnt++;
	return alarm;
}

Alarm AlarmUser::sleepFor(const Time &delay) {
	return sleepTill(TheClock + delay);
}

void AlarmUser::cancelAlarms() {
	if (thePendAlarmCnt)
		TheAlarmClock.cancelAll(this);
	Assert(!thePendAlarmCnt);
}


/* AlarmClock */

AlarmClock::AlarmClock() {
}

void AlarmClock::setAlarm(const Alarm &alarm) { 
	if (alarm)
		theQueue.add(alarm);
}

void AlarmClock::cancelAll(const AlarmUser *sleeper) {
	for (int i = theQueue.count()-1; i >= 0 && sleeper->alarmWaiting(); --i) {
		if (theQueue.at(i).sleeper() == sleeper)
			theQueue.at(i).snooze();
	}
}

// rings all ready alarms
void AlarmClock::ring() {
	// process alarms that were current at the call time only
	for (int c = theQueue.count(); c && !theQueue.empty(); --c) {
		if (const Alarm &alarm = theQueue.top()) {
			if (alarm.time() <= theCurTime) {
				static Time lastRing;
				Assert(lastRing <= alarm.time());
				lastRing = alarm.time();
				theQueue.shift().ring();
			} else
				break; // not ready yet
		} else {
			theQueue.skip();
		}
	}
}
		
void AlarmClock::update(Time curTime) {
	Clock::update(curTime);
	ring();
}

// must be called only for an alarm clock that is on
Time AlarmClock::timeLeft() const {
	Assert(!theQueue.empty());
	if (const Alarm &alarm = theQueue.top())
		return alarm.time() - theCurTime;
	Assert(false); // timeLeft() must be called only for set alarm clock
	return Time();
}

bool AlarmClock::on() {
	while (!theQueue.empty() && !theQueue.top())
		theQueue.skip();
	return !theQueue.empty();
}
