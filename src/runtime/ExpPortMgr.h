
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_EXPPORTMGR_H
#define POLYGRAPH__RUNTIME_EXPPORTMGR_H

#include "xstd/Ring.h"
#include "runtime/PortMgr.h"

// fifo pool of ports, explicit mapping with bind
class ExpPortMgr: public PortMgr {
	public:
		ExpPortMgr(const NetAddr &anAddr, int aPortMin, int aPortMax);

		void configure(int aPortMin, int aPortMax);

	protected:

		virtual int allocPort(Socket &s);
		virtual void freePort(int port, bool good);

		int findPort(Ring<int> &ports, Socket &s);
		bool bindToPort(Socket &s, int port) const;
		bool usedPort(int port) const;
};

#endif
