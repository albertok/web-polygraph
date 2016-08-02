
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_DUTWATCHDOG_H
#define POLYGRAPH__RUNTIME_DUTWATCHDOG_H

#include "runtime/StatIntvl.h"
#include "runtime/Goal.h"

class EveryCodeSym;
class GoalSym;
class Script;
class StatPhase;


// monitors DUT state and executes a corective action if needed
class DutWatchdog: public StatIntvl, public GoalSubj {
	friend class Script;

	public:
		static bool IsEnabled; // XXX: to disable on the server side

	public:
		DutWatchdog();
		virtual ~DutWatchdog();

		void configure(const EveryCodeSym &cfg);
		void configureGoal(const GoalSym &goalCfg);

		void start();
		void stop();

		virtual void wakeUp(const Alarm &alarm);

		// for GoalSubj
		virtual Time duration() const;
		virtual Counter xactCnt() const;
		virtual BigSize fillSz() const;
		virtual Counter fillCnt() const;
		virtual Counter xactErrCnt() const;

	protected:
		virtual StatIntvlRec &getRec(int cat);
		virtual const StatIntvlRec &getRec(int cat) const;
		virtual bool checkpoint();

		void act();
		void nextSample();

	protected:
		int theId;
		Goal *theGoal;       // compiled sampling goal
		GoalSym *theGoalSym; // configured sampling goal
		Script *theScript;   // what to do when state and goal are reached

		StatIntvlRec *theStats; // current stats
		Time theStart;          // when we started collecting the stats

	private:
		static int TheLastId;
};

#endif
