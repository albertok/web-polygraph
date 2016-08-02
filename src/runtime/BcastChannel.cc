
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Clock.h"
#include "runtime/BcastRcver.h"
#include "runtime/BcastChannel.h"

bool BcastChannel::Debug = false;

BcastChannel::BcastChannel(const String &anName, const bool willStopTrafficWaiting):
	stopsTrafficWaiting(willStopTrafficWaiting),
	theName(anName) {
}

void BcastChannel::subscribe(BcastRcver* rcver) {
	int idx;
	if (!find(rcver, idx))
		append(rcver);
}

void BcastChannel::unsubscribe(BcastRcver* rcver) {
	int idx;
	if (find(rcver, idx)) {
		// move to the tail and pop
		swap(idx, count()-1);
		pop();
	}
}

void BcastChannel::showEvent() const {
	clog << TheClock << " event: " << theName << " x " << count() << endl;
}
