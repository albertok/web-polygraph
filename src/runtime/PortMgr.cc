
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Socket.h"
#include "runtime/PortMgr.h"

LevelStat PortMgr::TheBoundLvl;


PortMgr::PortMgr(const NetAddr &anAddr): theAddr(anAddr) {
}

int PortMgr::bind(Socket &s) {
	const int port = allocPort(s);
	if (port >= 0) // success
		++TheBoundLvl;

	return port;
}

void PortMgr::release(int port, bool good) {
	Assert(port >= 0);
	freePort(port, good);
	--TheBoundLvl;
}
