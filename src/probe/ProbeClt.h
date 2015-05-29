
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_PROBECLT_H
#define POLYGRAPH__PROBE_PROBECLT_H

#include "xstd/AlarmClock.h"
#include "probe/ProbeAgent.h"

class ProbeCltXact;

class ProbeClt: public ProbeAgent, public AlarmUser {
	public:
		ProbeClt(const NetAddr &aLclHost, const NetAddr &aRmtHost);

		virtual void exec();

		void noteXactDone(ProbeCltXact *x);

		virtual void noteWriteReady(int fd);
		virtual void wakeUp(const Alarm &a);

	protected:
		void startConnect();
		void tryAgainLater();
};

#endif
