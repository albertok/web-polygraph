
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_PORTMGR_H
#define POLYGRAPH__RUNTIME_PORTMGR_H

#include "base/LevelStat.h"
#include "xstd/NetAddr.h"

class Socket;

// manages allocation of network ports
// see EphPortMgr and ExpPortMgr for concrete classes
class PortMgr {
	public:
		static const LevelStat &BoundLvl() { return TheBoundLvl; }

	public:
		PortMgr(const NetAddr &anAddr);
		virtual ~PortMgr() {}

		const NetAddr &addr() const { return theAddr; }

		int bind(Socket &s); // returns port number
		void release(int port, bool good);

	protected:
		virtual int allocPort(Socket &s) = 0;
		virtual void freePort(int port, bool good) = 0;

	protected:
		static LevelStat TheBoundLvl; // bound ports level

	protected:
		NetAddr theAddr;              // local address to bind to
};

#endif
