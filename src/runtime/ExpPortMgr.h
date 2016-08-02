
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_EXPPORTMGR_H
#define POLYGRAPH__RUNTIME_EXPPORTMGR_H

#include "xstd/h/stdint.h"
#include "xstd/Ring.h"
#include "runtime/PortMgr.h"

// fifo pool of ports, explicit mapping with bind
class ExpPortMgr: public PortMgr {
	public:
		explicit ExpPortMgr(const NetAddr &anAddr);

		static int PortMin();
		static int PortMax();
		static int PortCount() { return PortMax() - PortMin() + 1; }
		static bool Belongs(const int port) { return port >= PortMin() && port <= PortMax(); }

	protected:

		static void CheckUsedPorts(); // check for used ports on the system
		static bool UsedPort(int port);
		static bool UsedPort(const NetAddr &a);

	protected:
		typedef Ring<uint16_t> Ports;

		virtual int allocPort(Socket &s);
		virtual void freePort(int port, bool good);

		int findPort(Ports &ports, Socket &s);
		bool bindToPort(Socket &s, int port) const;

	protected:

		static Ports TheFreePorts; // free ports on the system
		static Ports TheUsedPorts; // used ports on the system

	protected:

		Ports theValidPorts; // never- or successfully-used ports
		Ports theVoidPorts; // last-use-failed ports
};

#endif
