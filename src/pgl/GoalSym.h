
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_GOALSYM_H
#define POLYGRAPH__PGL_GOALSYM_H

#include "xstd/Time.h"
#include "pgl/PglRecSym.h"

class BigSize;

// run goal specification
class GoalSym: public RecSym {
	public:
		static String TheType;

	public:
		GoalSym();
		GoalSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		Time duration() const;
		bool xactCount(Counter &count) const;
		bool fillSize(BigSize &sz) const;
		bool errs(double &ratio, Counter &count) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
