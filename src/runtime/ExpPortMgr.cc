
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits>

#include "xstd/h/iomanip.h"
#include "xstd/Socket.h"
#include "xstd/Rnd.h"
#include "base/polyLogCats.h"
#include "runtime/LogComment.h"
#include "runtime/SharedOpts.h"
#include "runtime/ErrorMgr.h"
#include "runtime/ExpPortMgr.h"

ExpPortMgr::Ports ExpPortMgr::TheFreePorts;
ExpPortMgr::Ports ExpPortMgr::TheUsedPorts;


ExpPortMgr::ExpPortMgr(const NetAddr &anAddr):
	PortMgr(anAddr) {
	theAddr.port(-1);

	if (PortMax() > numeric_limits<Ports::Value>::max()) {
		Comment(0) << "max value for '" << TheOpts.thePorts.name() <<
			"' option is " << numeric_limits<Ports::Value>::max() <<
			"; got ";
		TheOpts.thePorts.report(Comment);
		Comment << endc << xexit;
	}

	CheckUsedPorts();
	theValidPorts = TheFreePorts;
	theVoidPorts = TheUsedPorts;
	Must(theValidPorts.count() + theVoidPorts.count() == PortCount());

	// randomize the order in case there is a bad subrange or something...
	static RndGen rng;
	theValidPorts.randomize(rng);
	theVoidPorts.randomize(rng);
}

int ExpPortMgr::PortMin() {
	return TheOpts.thePorts.lo();
}

int ExpPortMgr::PortMax() {
	return TheOpts.thePorts.hi();
}

void ExpPortMgr::CheckUsedPorts() {
	if (!TheFreePorts.empty() || !TheUsedPorts.empty())
		return;

	Must(PortCount() > 0);
	TheFreePorts.resize(PortCount());
	TheUsedPorts.resize(PortCount());

	Comment(6) << "port mgr is scanning ports " << PortMin() << ':' << PortMax()
		<< " to find used ones" << endc;

	// push free ports, weed out used ones
	for (int p = PortMin(); p <= PortMax(); ++p) {
		if (UsedPort(p))
			TheUsedPorts.enqueue(p);
		else
			TheFreePorts.enqueue(p);
	}

	Comment(5) << "port mgr scanned " << PortCount() << " ports and found "
		<< TheUsedPorts.count() << " used ones" << endc;
}

bool ExpPortMgr::UsedPort(const int port) {
	return UsedPort(NetAddr(InAddress::IPvFour(), port)) ||
		UsedPort(NetAddr(InAddress::IPvSix(), port));
}

bool ExpPortMgr::UsedPort(const NetAddr &a) {
	Socket s;
	if (!s.create(a.addrN().family()))
		return false;
	const bool used = !s.bind(a);
	s.close();
	return used;
}

int ExpPortMgr::allocPort(Socket &s) {
	int port;

	port = findPort(theValidPorts, s);
	if (port < 0)
		port = findPort(theVoidPorts, s);

	if (port < 0) {
		Comment << here << "no ports left at " << theAddr << ";"
			<< " bound: " << BoundLvl().level()
			<< " valid: " << theValidPorts.count()
			<< " void:  " << theVoidPorts.count()
			<< " range: [" << PortMin() << ',' << PortMax() << ']'
			<< endc;
		Must(theValidPorts.empty());
		Error::Last(EADDRNOTAVAIL);
	} else {
		Must(Belongs(port));
	}
	return port;
}

void ExpPortMgr::freePort(int port, bool good) {
	Must(Belongs(port));
	if (good)
		theValidPorts.enqueue(port);
	else
		theVoidPorts.enqueue(port);
}

int ExpPortMgr::findPort(Ports &ports, Socket &s) {
	// note: if ports == theVoidPorts, ports may "grow" inside the loop
	for (int i = ports.count(); i > 0; --i) {
		const int port = ports.dequeue();
		Must(Belongs(port));
		if (!bindToPort(s, port)) {
			if (ReportError2(Error::Last(), lgcCltSide))
				Comment << "failed to bind to " << theAddr << ':' << port << endc;
			freePort(port, false);
		} else
			return port;
	}
	return -1;
}

bool ExpPortMgr::bindToPort(Socket &s, int port) const {
	NetAddr a(theAddr.addrN(), port);
	return s.bind(a);
}
