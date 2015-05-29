
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/polyBcastChannels.h"
#include "probe/ProbeXact.h"
#include "probe/PolyProbe.h"
#include "probe/ProbeStatMgr.h"

ProbeXact::ProbeXact(const NetAddr &aCltHost,const NetAddr &aSrvHost, int fd):
	theCltHost(aCltHost), theSrvHost(aSrvHost), theSock(fd), theStats(0), theRdCount(0) {
}

ProbeXact::~ProbeXact() {
	if (theReadR)
		TheFileScanner->clearRes(theReadR);
	if (theWriteR)
		TheFileScanner->clearRes(theWriteR);
	if (theSock.fd() >= 0)
		theSock.close();
}

void ProbeXact::exec() {
	theReadR = TheFileScanner->setFD(theSock.fd(), dirRead, this);
	theWriteR = TheFileScanner->setFD(theSock.fd(), dirWrite, this);

	theStats = TheProbeStatMgr.stats(cltHost(), srvHost());
	Assert(theStats);

	theChannels.append(ThePhasesEndChannel);
	startListen();
}

void ProbeXact::finish(const Error &err) {
	Assert(owner());
	if (err)
		cerr << cltHost() << "<->" << srvHost() << " : error: " << err << endl;
	owner()->noteXactDone(this); // will destroy
}

void ProbeXact::noteReadReady(int fd) {
	Assert(fd == theSock.fd()); // remove later

	static char buf[16*1024];
	buf[0] = (char)0;
	const Size sz = theSock.read(buf, sizeof(buf));
	theRdCount++;

	if (sz < 0) {
		finish(Error::Last());
	} else
	if (sz == 0) {
		finish(Error::None());
	} else 
	if (theRdCount == 1 && buf[0]) {
		ThePolyProbe->sendStats(theSock, theCltHost);
		finish(Error::None());
	} else {
		theStats->recordRead(sz);
	}
}

void ProbeXact::noteWriteReady(int fd) {
	Assert(fd == theSock.fd()); // remove later

	static char buf[16*1024];
	const Size sz = theSock.write(buf, sizeof(buf));

	if (sz < 0)
		finish(Error::Last());
	else
		theStats->recordWrite(sz);
}

void ProbeXact::noteInfoEvent(BcastChannel *ch, InfoEvent ev) {
	Assert(ch == ThePhasesEndChannel);
	Assert(ev == BcastRcver::ieNone);
	if (theSock.fd() >= 0) {
		theSock.close();
		TheFileScanner->clearRes(theReadR);
		TheFileScanner->clearRes(theWriteR);
	}
	finish(Error::None());
}
