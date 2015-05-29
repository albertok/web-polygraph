
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "probe/ProbeClt.h"
#include "probe/ProbeCltXact.h"


ProbeCltXact::ProbeCltXact(const NetAddr &aCltHost, const NetAddr &aSrvHost, int fd):
	ProbeXact(aCltHost, aSrvHost, fd) {
}

ProbeAgent *ProbeCltXact::owner() {
	return theOwner;
}

const ProbeAgent *ProbeCltXact::owner() const {
	return theOwner;
}

void ProbeCltXact::exec(ProbeClt *anOwner) {
	theOwner = anOwner;
	ProbeXact::exec();
	noteWriteReady(theWriteR.fd()); // assume we are ready to write
}
