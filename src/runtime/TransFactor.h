
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_TRANSFACTOR_H
#define POLYGRAPH__RUNTIME_TRANSFACTOR_H

#include "xstd/String.h"

class StatPhase;
class GoalRec;

class TransFactor {
	public:
		TransFactor(const StatPhase *aPhase, const String &aName);

		void configure(const double beg, const double end, const bool isFirst);
		void noteStart(const TransFactor *const prev);

		double beg() const { return theBeg; }
		double end() const { return theEnd; }
		bool flat() const { return !theK; }

		double current() const;
		double current(Time lastTm, int lastCnt = -1) const;

		void changeBy(double delta);
		void setTo(double level);

		void reportCfg(ostream &os) const;

	protected:
		const GoalRec &goal() const;
		void change(double &curVal, double newVal, const char *kind);

	protected:
		const StatPhase *thePhase;
		const String theName;

		double theBeg;
		double theEnd;
		double theK;
};


#endif
