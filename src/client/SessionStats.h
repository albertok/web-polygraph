
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
		virtual Counter xactCnt() const;
		virtual BigSize fillSz() const;
		virtual Counter fillCnt() const;
		virtual Counter xactErrCnt() const;

	public:
		Time theStart;
		Counter theXactCnt;
		BigSize theFillSz;
		Counter theFillCnt;
		Counter theXactErrCnt;
};

#endif
