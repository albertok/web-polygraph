
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_SESSIONSYM_H
#define POLYGRAPH__PGL_SESSIONSYM_H

#include "pgl/PglRecSym.h"

class Time;
class RndDistr;
class GoalSym;

// base class for robot and server symbols
class SessionSym: public RecSym {
	public:
		static const String TheType;

	public:
		SessionSym();
		SessionSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		GoalSym *busyPeriod() const;
		RndDistr *idlePeriodDuration() const;
		bool heartbeatGap(Time &gap) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
