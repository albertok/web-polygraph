
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_FTPCXM_H
#define POLYGRAPH__CLIENT_FTPCXM_H

#include "client/CltXactMgr.h"

class CltXact;
class FtpCltXact;
class Connection;

// FtpCltXact I/O manager
// similar to SingleCxm used for HttpCltXact but supports rwrwrwr sequences
class FtpCxm: public CltXactMgr {
	public:
		static FtpCxm *Get();

	public:
		FtpCxm();

		virtual void reset();

		virtual bool pipelining() const;

		void releaseReadControl(CltXact *x);
		virtual void control(CltXact *x);
		virtual void release(CltXact *x);

		// resume writing after a pause
		virtual void resumeWriting(CltXact *x);

		virtual void noteAbort(CltXact *x);
		virtual void noteDone(CltXact *x);
		virtual void noteLastXaction(CltXact *x);

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);

		virtual Connection *conn();

	private:
		FtpCltXact *theXact;
		bool needMoreFill;
};

#endif
