
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "probe/ProbeSrv.h"
#include "probe/ProbeSrvXact.h"
//#include "probe/PolyProbe.h"

ProbeSrvXact::ProbeSrvXact(const NetAddr &aCltHost, const NetAddr &aSrvHost, int fd):
	ProbeXact(aCltHost, aSrvHost, fd) {
}

ProbeAgent *ProbeSrvXact::owner() {
	return theOwner;
}

const ProbeAgent *ProbeSrvXact::owner() const {
	return theOwner;
}

void ProbeSrvXact::exec(ProbeSrv *anOwner) {
	theOwner = anOwner;
	ProbeXact::exec();
}
