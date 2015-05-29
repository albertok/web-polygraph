
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_SINGLECXM_H
#define POLYGRAPH__CLIENT_SINGLECXM_H

#include "client/CltXactMgr.h"

class CltXact;
class Connection;

// a manager of a single CltXact that does not share the connection
// with other client transactions; supports a "wwwwrrrrr" I/O sequence
class SingleCxm: public CltXactMgr {
	public:
		static SingleCxm *Get();

	public:
		SingleCxm();

		virtual void reset();

		virtual bool pipelining() const;

		void releaseReadControl(CltXact *x);
		virtual void control(CltXact *x);
		virtual void release(CltXact *x);

		virtual void resumeWriting(CltXact *);

		virtual void noteAbort(CltXact *x);
		virtual void noteDone(CltXact *x);
		virtual void noteLastXaction(CltXact *x);

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);

		virtual Connection *conn();

	private:
		CltXact *theXact;
		bool needMoreFill;
};

#endif
