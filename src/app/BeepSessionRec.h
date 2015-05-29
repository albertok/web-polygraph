
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__APP_BEEPSESSIONREC_H
#define POLYGRAPH__APP_BEEPSESSIONREC_H

#include "xstd/FileScanner.h"
#include "xstd/Socket.h"

class BeepSessionMgr;

// keeps state for BeepDoorMan's sessions
class BeepSessionRec {
	public:
		BeepSessionRec();

	public:
		int theIdx;
		Socket theSock;
		NetAddr theRAddr;        // remote address
		BeepSessionMgr *theMgr;
		FileScanReserv theRdRes;
		FileScanReserv theWrRes;
};

#endif
