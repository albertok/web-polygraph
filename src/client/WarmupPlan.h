
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_WARMUPPLAN_H
#define POLYGRAPH__CLIENT_WARMUPPLAN_H

#include "xstd/Ring.h"


// iterates through vidx values until all reachable pub worlds are ready
class WarmupPlan {
	public:
		static int ReadyCount();

	protected:
		static bool WarmViserv(int vidx);
		static void ReportProgress();

	protected:
		static int TheReqCnt;


	public:
		WarmupPlan(const Array<int> &visIdx);
		~WarmupPlan();

		int planCount() const { return thePlan.count(); };
		int selectViserv();

	protected:
		Ring<int> thePlan;
};

#endif
