
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/PassClt.h"

PassClt::PassClt() {
	Assert(!theThinkDistr);
}

void PassClt::wakeUp(const Alarm &a) {
	Client::wakeUp(a);
}

void PassClt::scheduleLaunch(Time) {
	// no launches -- fetching objects on demand only
}

bool PassClt::launchCanceled(CltXact *x) {
	return Client::launchCanceled(x);
}
