
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_SESSIONSTATS_H
#define POLYGRAPH__CLIENT_SESSIONSTATS_H

#include "runtime/Goal.h"

class SessionStats: public GoalSubj {
	public:
		SessionStats();

		void restart();

		// finalize GoalSubj interface
		virtual Time duration() const;
		virtual int xactCnt() const;
		virtual BigSize fillSz() const;
		virtual int fillCnt() const;
		virtual int xactErrCnt() const;

	public:
		Time theStart;
		int theXactCnt;
		BigSize theFillSz;
		int theFillCnt;
		int theXactErrCnt;
};

#endif
