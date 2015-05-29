
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Socket.h"
#include "base/polyLogCats.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/EphPortMgr.h"


EphPortMgr::EphPortMgr(const NetAddr &anAddr): PortMgr(anAddr) {
	theAddr.port(0);
}

int EphPortMgr::allocPort(Socket &s) {
	if (s.bind(theAddr))
		return s.lport();

	if (ReportError2(Error::Last(), lgcCltSide)) {
		ostream &os = Comment << "OS probably ran out of ephemeral ports";
		if (theAddr)
			Comment << " at " << theAddr;
		os << endc;
	}
	return -1;
}

void EphPortMgr::freePort(int, bool) {
}
