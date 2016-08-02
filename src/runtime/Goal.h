
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_GOAL_H
#define POLYGRAPH__RUNTIME_GOAL_H

#include "base/GoalRec.h"

class GoalSym;
class GoalSubj;

// run-time wrapper for GoalRec
class Goal: public GoalRec {
	public:
		static Counter MinXactForRatios; // min #xacts for ratios to make sense

	public:
		Goal();

		void configure(const GoalSym &cfg);
		bool reached(const GoalSubj &subj) const;
		bool reachedPositive(const GoalSubj &subj) const;
		bool reachedNegative(const GoalSubj &subj) const;

		void reportProgress(ostream &os, const GoalSubj &s) const;
};

// base class for objects fed to Goal::reached()
class GoalSubj {
	public:
		virtual ~GoalSubj();

		virtual Time duration() const = 0;
		virtual Counter xactCnt() const = 0;
		virtual BigSize fillSz() const = 0;
		virtual Counter fillCnt() const = 0;
		virtual Counter xactErrCnt() const = 0;

		double xactErrRatio() const;
};

#endif
