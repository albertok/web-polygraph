
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Socket.h"
#include "runtime/EphPortMgr.h"
#include "runtime/ExpPortMgr.h"
#include "runtime/PortMgr.h"
#include "runtime/SharedOpts.h"

LevelStat PortMgr::TheBoundLvl;
PtrArray<PortMgr*> PortMgr::ThePortMgrs;


// slow, used during configuration only
PortMgr *PortMgr::Get(NetAddr addr) {
	addr.port(-1); // ignore port

	// slowly check if we already have a port mgr for the given address
	for (int i = 0; i < ThePortMgrs.count(); ++i) {
		if (ThePortMgrs[i]->addr() == addr)
			return ThePortMgrs[i];
	}

	// create a new port manager
	PortMgr *const mgr = TheOpts.thePorts.set() ?
		static_cast<PortMgr *>(new ExpPortMgr(addr)) :
		static_cast<PortMgr *>(new EphPortMgr(addr));
	ThePortMgrs.append(mgr);
	return mgr;
}

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
