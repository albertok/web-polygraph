
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "runtime/ErrorMgr.h"
#include "runtime/IcpXaction.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "base/polyLogCats.h"


void IcpXaction::reset() {
	theAddr = NetAddr();
	theOid.reset();
	theStartTime = theLifeTime = Time();
	theRepSize = Size();
	theResult = icpInvalid;
	theLogCat = lgcAll;
}

void IcpXaction::start() {
	theStartTime = TheClock;
}

void IcpXaction::finish(Error err) {
	theLifeTime = TheClock - theStartTime;

	if (err) {
		theResult = icpInvalid;
		if (err != errOther)
			ReportError(err);
	} else {
		Broadcast(TheIcpXactEndChannel, this);
	}
}
