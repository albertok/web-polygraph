
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/Farm.h"
#include "runtime/LogComment.h"
#include "runtime/globals.h"
#include "client/CltXact.h"
#include "client/SingleCxm.h"

static ObjFarm<SingleCxm> TheMgrs;

SingleCxm *SingleCxm::Get() {
	TheMgrs.limit(1024); // magic, no good way to estimate
	return TheMgrs.get();
}

SingleCxm::SingleCxm(): theXact(0), needMoreFill(true) {
}

void SingleCxm::reset() {
	theXact = 0;
	needMoreFill = true;
	CltXactMgr::reset();
}

bool SingleCxm::pipelining() const {
	return false;
}

Connection *SingleCxm::conn() {
	return theXact ? theXact->conn() : 0;
}

void SingleCxm::release(CltXact *x) {
	Assert(theXact == x);
	TheMgrs.put(this);
}

void SingleCxm::control(CltXact *x) {
	Assert(!theXact && x);
	theXact = x;
	theWrSize = 0;
	needMoreFill = true;
	theXact->conn()->theWr.start(this);
}

void SingleCxm::noteAbort(CltXact *x) {
	noteDone(x);
}

void SingleCxm::noteDone(CltXact *x) {
	Assert(theXact == x);
	conn()->theWr.stop(this);
	conn()->theRd.stop(this);
}

void SingleCxm::noteLastXaction(CltXact *) {
	// nothing to be done
}

void SingleCxm::resumeWriting(CltXact *x) {
	Must(theXact == x);
	Must(!conn()->theWr.theReserv);
	needMoreFill = true;
	conn()->theWr.start(this);
	TheFileScanner->setPriority(conn()->sock().fd(), fsupBestEffort);
}

void SingleCxm::noteReadReady(int) {
//Comment << here << "SingleCxm::noteReadReady" << endc;
	Assert(theXact);
	if (!theXact->controlledMasterRead())
		return;
//Comment << here << "controlledMasterRead successful" << endc;
	// calls us back if finished; otherwise, waits for more read I/O
	bool needMoreRead = false; // XXX: unused for HTTP
	theXact->controlledPostRead(needMoreRead);
}

void SingleCxm::noteWriteReady(int) {
	Assert(theXact);

//Comment << here << "noteWriteReady; needMoreFill=" << needMoreFill << endc;

	if (needMoreFill &&
		!theXact->controlledFill(needMoreFill))
		return;

//Comment << here << "controlledFilled" << endc;

	if (!theXact->controlledMasterWrite(theWrSize))
		return;

//Comment << here << "controlledMasterWrote " << theWrSize << endc;

	bool needMoreWrite = true;
	if (!theXact->controlledPostWrite(theWrSize, needMoreWrite))
		return;

//Comment << here << "controlledPostWrore " << theWrSize << ", " << needMoreWrite << endc;

	if (needMoreWrite) {
		if (!conn()->theWr.theReserv)
			conn()->theWr.start(this);
	} else {
		conn()->theWr.stop(this);
		if (!conn()->theRd.theReserv) {
			conn()->theRd.start(this);
			TheFileScanner->setPriority(conn()->sock().fd(), fsupBestEffort);
		}
	}
//Comment << here << "SingleCxm::noteWriteReady I/O: r" << conn()->theRd.theReserv << " w" << conn()->theWr.theReserv << endc;
}
