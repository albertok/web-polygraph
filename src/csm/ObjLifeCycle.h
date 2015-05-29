
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CSM_OBJLIFECYCLE_H
#define POLYGRAPH__CSM_OBJLIFECYCLE_H

#include "xstd/Array.h"
#include "xstd/Rnd.h"

class ObjTimes;
class ObjLifeCycleSym;

// models object modification and expected expiration times
class ObjLifeCycle {
	protected:
		enum ExpGapType { egtNone, egtLmt, egtNow, egtNmt };

	public:
		ObjLifeCycle();
		~ObjLifeCycle();

		void configure(const ObjLifeCycleSym *cfg);

		void calcTimes(int seed, ObjTimes &times);

	protected:
		int lmtDelta(int seed, int cycleCnt, int cycleLen);
		int gapType(int qualifSymKind) const;

	protected:
		RndGen theRng;
		RndDistr *theModTime;
		Array<RndDistr*> theExpires;
		RndDistr *theExpSelector;
		Array<ExpGapType> theExpGapTypes;

		double theModVar;        // "variation" in lmt
		double theShowModRatio;  // how many objs know their mod time
};

#endif
