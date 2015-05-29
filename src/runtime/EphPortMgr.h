
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_EPHPORTMGR_H
#define POLYGRAPH__RUNTIME_EPHPORTMGR_H

#include "runtime/PortMgr.h"

// ephemeral port manager; simply calls bind(0) to allocate a port
class EphPortMgr: public PortMgr {
	public:
		EphPortMgr(const NetAddr &anAddr);

	protected:
		virtual int allocPort(Socket &s);
		virtual void freePort(int port, bool good);
};

#endif
