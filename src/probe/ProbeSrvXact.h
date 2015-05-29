
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_PROBESRVXACT_H
#define POLYGRAPH__PROBE_PROBESRVXACT_H

#include "probe/ProbeXact.h"

class ProbeSrv;

class ProbeSrvXact: public ProbeXact {
	public:
		ProbeSrvXact(const NetAddr &aCltHost, const NetAddr &aSrvHost, const int fd);

		void exec(ProbeSrv *anOwner);
		virtual ProbeAgent *owner();
		virtual const ProbeAgent *owner() const;

	protected:
		ProbeSrv *theOwner;
};

#endif
