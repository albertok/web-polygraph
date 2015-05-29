
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_PIPELINEDCXM_H
#define POLYGRAPH__CLIENT_PIPELINEDCXM_H

#include "xstd/Queue.h"
#include "client/CltXactMgr.h"

class CltXact;
class Connection;

// a manager of a single CltXact that does not share the connection
// with other client transactions
class PipelinedCxm: public CltXactMgr {
	public:
		static PipelinedCxm *Get();

	public:
		PipelinedCxm();

		virtual void reset();

		virtual bool pipelining() const;

		void assumeReadControl(CltXact *x, CltXactMgr *oldMgr);
		virtual void join(CltXact *x);
		virtual void control(CltXact *x);
		virtual void release(CltXact *x); // opposite of Get()

		virtual void resumeWriting(CltXact *);

		virtual void noteAbort(CltXact *x);
		virtual void noteDone(CltXact *x);
		virtual void noteLastXaction(CltXact *x);

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);

		virtual Connection *conn();

	protected:
		typedef Queue<CltXact, &CltXact::pipelinedXacts> Line;
		void prepReading(CltXact *x);
		void abortLines(CltXact *cause);
		void abortLine(Line &line, CltXact *cause);
		void kickNextRead();

	protected:
		Line theFillers;
		Line theWriters;
		Line theReaders;
		Connection *theConn; // cached value
		int theUseLevel;
};

#endif
