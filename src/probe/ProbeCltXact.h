
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PROBE_PROBECLTXACT_H
#define POLYGRAPH__PROBE_PROBECLTXACT_H

#include "probe/ProbeXact.h"

class ProbeClt;

class ProbeCltXact: public ProbeXact {
	public:
		ProbeCltXact(const NetAddr &aCltHost, const NetAddr &aSrvHost, const int fd);

		void exec(ProbeClt *anOwner);
		virtual ProbeAgent *owner();
		virtual const ProbeAgent *owner() const;


	protected:
		ProbeClt *theOwner;
};

#endif
