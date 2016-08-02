
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/Farm.h"
#include "runtime/globals.h"
#include "client/CltXact.h"
#include "client/PipelinedCxm.h"

static ObjFarm<PipelinedCxm> TheMgrs;

PipelinedCxm *PipelinedCxm::Get() {
	TheMgrs.limit(1024); // magic, no good way to estimate
	return TheMgrs.get();
}

PipelinedCxm::PipelinedCxm(): theConn(0), theUseLevel(0) {
}

void PipelinedCxm::reset() {
	theConn = 0;
	theUseLevel = 0;
	CltXactMgr::reset();
}

bool PipelinedCxm::pipelining() const {
	return true;
}

Connection *PipelinedCxm::conn() {
	Assert(theConn);
	return theConn;
}

void PipelinedCxm::assumeReadControl(CltXact *x, CltXactMgr *oldMgr) {
	++theUseLevel;
	theConn = x->conn();
	theConn->theRd.changeUser(oldMgr, this);
	theReaders.enqueue(x);
}

void PipelinedCxm::join(CltXact *) {
	++theUseLevel;
	// x is expected to call control() or release() eventually
}

void PipelinedCxm::control(CltXact *x) {
	theFillers.enqueue(x);
	if (!conn()->theWr.theReserv)
		conn()->theWr.start(this);
}

void PipelinedCxm::release(CltXact *) {
	if (--theUseLevel <= 0)
		TheMgrs.put(this);
}

void PipelinedCxm::noteAbort(CltXact *x) {
	abortLines(x);
	conn()->theWr.stop(this);
	conn()->theRd.stop(this);
}

// we assume that done transactions are all readers
void PipelinedCxm::noteDone(CltXact *x) {
	Assert(theReaders.count() > 0);
	theReaders.dequeue(x);

	if (theReaders.count() == 0)
		conn()->theRd.stop(this);
	else
		kickNextRead(); // kick the next one
}

void PipelinedCxm::noteLastXaction(CltXact *x) {
	// for simplicity, assume that the current reader found the end
	if (!Should(theReaders.count() > 0 && x == theReaders.begin()))
		return;
	abortLines(x); // deletes x so we reinsert it
	theReaders.enqueue(x);
	conn()->theWr.stop(this);
}

void PipelinedCxm::resumeWriting(CltXact *) {
	Must(!conn()->theWr.theReserv);
	conn()->theWr.start(this);
	TheFileScanner->setPriority(conn()->sock().fd(), fsupBestEffort);
}

void PipelinedCxm::noteReadReady(int) {
	Assert(theReaders.count() > 0);
	// will call us back
	theReaders.begin()->controlledMasterRead();
}

void PipelinedCxm::noteWriteReady(int) {
	CltXact *next = 0;
	for (CltXact *x = theFillers.begin(); x != theFillers.end() && !conn()->theWrBuf.full(); x = next) {
		bool needMoreFill = false;
		if (!x->controlledFill(needMoreFill))
			return;
		if (theWriters.empty() ||
			theWriters.lastIn() != x)
			theWriters.enqueue(x);
		if (needMoreFill)
			break;
		next = theFillers.next(x);
		theFillers.dequeue(x);
	}

	// write once
	if (theWriters.count() > 0) {
		if (!theWriters.begin()->controlledMasterWrite(theWrSize))
			return;
	}

	// one or more transactions may move on after a write
	for (CltXact *w = theWriters.begin(); w != theWriters.end() && theWrSize > 0; w = next) {
		bool needMoreWrite = false;
		if (!w->controlledPostWrite(theWrSize, needMoreWrite))
			return;

		if (theReaders.empty() ||
			theReaders.lastIn() != w)
			prepReading(w);
		if (theReaders.count() == 1)
			kickNextRead();

		if (needMoreWrite) {
			if (!conn()->theWr.theReserv)
				conn()->theWr.start(this);
			return;
		}

		next = theWriters.next(w);

		theWriters.dequeue(w);
		if (theFillers.count() == 0 && theWriters.count() == 0)
			conn()->theWr.stop(this);
	}
}

void PipelinedCxm::kickNextRead() {
	if (!theReaders.empty() && conn()->theRdBuf.contSize() > 0) {
		CltXact *x = theReaders.begin();
		bool needMoreRead = false; // XXX: unused for HTTP
		// calls us back if finished; otherwise, waits for more read I/O
		x->controlledPostRead(needMoreRead);
	}
}

void PipelinedCxm::prepReading(CltXact *x) {
	theReaders.enqueue(x);
	if (!conn()->theRd.theReserv) {
		conn()->theRd.start(this);
		TheFileScanner->setPriority(conn()->sock().fd(), fsupBestEffort);
	}
}

void PipelinedCxm::abortLine(Line &line, CltXact *cause) {
	while (!line.empty()) {
		CltXact *x = line.begin();
		if (x != cause)
			x->controlledAbort();
		line.dequeue(x); // including cause
	}
}

void PipelinedCxm::abortLines(CltXact *cause) {
	abortLine(theFillers, cause);
	abortLine(theWriters, cause);
	abortLine(theReaders, cause);
}
