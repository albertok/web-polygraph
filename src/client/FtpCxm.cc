
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/Farm.h"
#include "runtime/globals.h"
#include "client/FtpCltXact.h"
#include "client/FtpCxm.h"

static ObjFarm<FtpCxm> TheMgrs;

FtpCxm *FtpCxm::Get() {
	TheMgrs.limit(1024); // magic, no good way to estimate
	return TheMgrs.get();
}

FtpCxm::FtpCxm(): theXact(0), needMoreFill(true) {
}

void FtpCxm::reset() {
	theXact = 0;
	needMoreFill = true;
	CltXactMgr::reset();
}

bool FtpCxm::pipelining() const {
	return false;
}

Connection *FtpCxm::conn() {
	return theXact ? theXact->conn() : 0;
}

void FtpCxm::release(CltXact *x) {
	Assert(theXact == x);
	TheMgrs.put(this);
}

void FtpCxm::control(CltXact *x) {
	Assert(!theXact && x);
	theXact = dynamic_cast<FtpCltXact*>(x);
	Assert(theXact);
	theWrSize = 0;
	theXact->conn()->theRd.start(this);
}

void FtpCxm::noteAbort(CltXact *x) {
	noteDone(x);
}

void FtpCxm::noteDone(CltXact *x) {
	Assert(theXact == x);
	conn()->theWr.stop(this);
	conn()->theRd.stop(this);
}

void FtpCxm::noteLastXaction(CltXact *) {
	Must(false); // should not be called
}

void FtpCxm::resumeWriting(CltXact *x) {
	Must(theXact == x);
	Must(!conn()->theWr.theReserv);
	Must(theXact->wantsToWrite());
	needMoreFill = true;
	conn()->theWr.start(this);
	TheFileScanner->setPriority(conn()->sock().fd(), fsupBestEffort);
}

void FtpCxm::noteReadReady(int) {
	Assert(theXact);

	// calls us back if finished; otherwise, waits for more read I/O
	if (!theXact->controlledMasterRead())
		return;

	bool needMoreRead = false;
	if (!theXact->controlledPostRead(needMoreRead))
		return;

	if (needMoreRead) {
		if (!conn()->theRd.theReserv)
			conn()->theRd.start(this);
	} else {
		conn()->theRd.stop(this);
		if (!conn()->theWr.theReserv && theXact->wantsToWrite()) {
			needMoreFill = true;
			conn()->theWr.start(this);
			TheFileScanner->setPriority(conn()->sock().fd(), fsupBestEffort);
		}
	}
}

void FtpCxm::noteWriteReady(int) {
	Assert(theXact);

	if (needMoreFill &&
		!theXact->controlledFill(needMoreFill))
		return;

	if (!theXact->controlledMasterWrite(theWrSize))
		return;

	bool needMoreWrite = true;
	if (!theXact->controlledPostWrite(theWrSize, needMoreWrite))
		return;

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
}
