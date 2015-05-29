
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "probe/ProbeCltXact.h"
#include "probe/ProbeClt.h"
#include "probe/ProbeStatMgr.h"
#include "probe/ProbeOpts.h"
#include "probe/PolyProbe.h"

ProbeClt::ProbeClt(const NetAddr &aLclHost, const NetAddr &aRmtHost):
	ProbeAgent(aLclHost, aRmtHost) {
}

void ProbeClt::exec() {
	ProbeAgent::exec();
	// give servers time to start
	tryAgainLater();
}

void ProbeClt::startConnect() {
	Assert(theLclHost && theRmtHost);

	if (isDone)
		return;

	Must(theSock.create(theLclHost.addrN().family()));
	setSockOpt(theSock);
	Must(theSock.bind(theLclHost));

	clog << theLclHost << ": connecting to " << theRmtHost << endl;

	if (theSock.connect(theRmtHost)) {
		theReserv = TheFileScanner->setFD(theSock.fd(), dirWrite, this);
		TheProbeStatMgr.stats(theLclHost, theRmtHost)->recordConn();
	} else {
		TheProbeStatMgr.stats(theLclHost, theRmtHost)->recordError(Error::Last());
		theSock.close();
		tryAgainLater();
	}
}

void ProbeClt::noteXactDone(ProbeCltXact *x) {
	ProbeAgent::noteXactDone(x);
	startConnect();
}

void ProbeClt::noteWriteReady(int fd) {
	Assert(fd == theSock.fd()); // remove later
	TheFileScanner->clearRes(theReserv);
	theSock = -1; // we will not own the socket from now own
	ProbeCltXact *x = new ProbeCltXact(theLclHost, theRmtHost, fd);
	x->exec(this);
	// unsafe to call anything else as exec might return
}

void ProbeClt::tryAgainLater() {
	Assert(!theReserv);
	Assert(!thePendAlarmCnt);
	sleepFor(TheProbeOpts.theContTout);
}

void ProbeClt::wakeUp(const Alarm &a) {
	Assert(!theReserv);
	AlarmUser::wakeUp(a);
	startConnect();
}
