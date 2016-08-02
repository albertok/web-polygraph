
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "base/OLog.h"
#include "base/ILog.h"
#include "base/Progress.h"

Progress TheProgress;


Progress::Progress():
	socks("socks", "SOCKS"), ssl("ssl", "SSL"), ftp("ftp", "FTP"),
	connect("connect", "HTTP CONNECT"),
	authing("auth-ing", "Authenticating") {
	theXactCnt = theErrCnt = 0;
}

Time Progress::time() const {
	return theTimestamp < 0 ? TheClock.time() : theTimestamp;
}

void Progress::store(OLog &ol) const {
	ol << TheClock.time() << theXactCnt << theErrCnt;
}

void Progress::load(ILog &il) {
	il >> theTimestamp >> theXactCnt >> theErrCnt;
}

ostream &Progress::print(ostream &os) const {
	os << theXactCnt << ':';
	if (theTimestamp < 0)
		os << (TheClock - Clock::TheStartTime);
	else
		os << (theTimestamp - Clock::TheStartTime);
	return os << ':' << theErrCnt;
}
