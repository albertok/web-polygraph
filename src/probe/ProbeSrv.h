
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_PROBESRV_H
#define POLYGRAPH__PROBE_PROBESRV_H

#include "probe/ProbeAgent.h"

class ProbeSrvXact;

class ProbeSrv: public ProbeAgent {
	public:
		ProbeSrv(const NetAddr &aLclHost);

		virtual void exec();

		virtual void noteXactDone(ProbeSrvXact *x);
		virtual void noteReadReady(int fd);
};

#endif
