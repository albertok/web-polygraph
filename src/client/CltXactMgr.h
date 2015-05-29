
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CLTXACTMGR_H
#define POLYGRAPH__CLIENT_CLTXACTMGR_H

#include "xstd/FileScanner.h"

class CltXact;
class Connection;

// interface for managing of CltXact actions that may depend on other CltXacts
// essential for supporting pipelined messages "transparently" to CltXact
// TODO: CltXact does not use this protocol-agnostic interface much; split?
class CltXactMgr: public FileScanUser {
	public:
		CltXactMgr(): theWrSize(0) {}
		virtual ~CltXactMgr() {}

		virtual void reset() { theWrSize = 0; }

		virtual bool pipelining() const = 0; // true for PipelinedXactMgr only

		virtual void control(CltXact *x) = 0;  // start filling,writing,reading
		virtual void release(CltXact *x) = 0; // delete or put self into Farm

		// resume writing after a pause
		virtual void resumeWriting(CltXact *x) = 0;

		virtual void noteAbort(CltXact *x) = 0;
		virtual void noteDone(CltXact *x) = 0;
		virtual void noteLastXaction(CltXact *x) = 0; // conn is closing

		virtual void noteReadReady(int fd) = 0;
		virtual void noteWriteReady(int fd) = 0;

		virtual Connection *conn() = 0;

	protected:
		Size theWrSize;
};

#endif
