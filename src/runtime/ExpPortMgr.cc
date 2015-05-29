
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Socket.h"
#include "xstd/Rnd.h"
#include "base/polyLogCats.h"
#include "runtime/LogComment.h"
#include "runtime/SharedOpts.h"
#include "runtime/ErrorMgr.h"
#include "runtime/ExpPortMgr.h"

class PortHistory {
	friend class ExpPortMgr;

	public:
		PortHistory();

		void configure(int aPortMin, int aPortMax);

		bool configured() const;
		bool belongs(int port) const;

		void record(int port, bool good);

	protected:
		bool usedPort(int port) const;
		bool usedPort(const NetAddr &a) const;

	protected:
		Ring<int> theValidPorts; // never- or successfully-used ports
		Ring<int> theVoidPorts;  // last-use-failed ports

		int thePortMin;
		int thePortMax;
};

static PortHistory TheHistory;



/* PortHistory */

PortHistory::PortHistory(): thePortMin(-1), thePortMax(-1) {
}

void PortHistory::configure(int aPortMin, int aPortMax) {
	Assert(!theValidPorts.count());

	thePortMin = aPortMin;
	thePortMax = aPortMax;

	const int count = thePortMax - thePortMin + 1;
	Assert(count >= 0);
	theValidPorts.resize(count);
	theVoidPorts.resize(count);

	Comment(6) << "port mgr is scanning ports " << thePortMin << ':' << thePortMax << " to find used ones" << endc;

	// push free ports, weed out used ones
	for (int p = thePortMin; p <= thePortMax; ++p)
		record(p, !usedPort(p));

	Comment(5) << "port mgr scanned " << count << " ports"
		<< " and found " << theVoidPorts.count() << " used ones" << endc;

	Assert(theValidPorts.count() + theVoidPorts.count() == count);

	// randomize the order in case there is a bad subrange or something...
	static RndGen rng;
	theValidPorts.randomize(rng);
	theVoidPorts.randomize(rng);
}

bool PortHistory::configured() const {
	return theValidPorts.count() + theVoidPorts.count() > 0;
}

void PortHistory::record(int port, bool good) {
	Assert(port >= thePortMin && port <= thePortMax);
	if (good)
		theValidPorts.enqueue(port);
	else
		theVoidPorts.enqueue(port);
}

bool PortHistory::belongs(int port) const {
	return port >= thePortMin && port <= thePortMax;
}

bool PortHistory::usedPort(int port) const {
	return 
		usedPort(NetAddr(InAddress::IPvFour(), port)) || 
		usedPort(NetAddr(InAddress::IPvSix(), port));
}

bool PortHistory::usedPort(const NetAddr &a) const {
	Socket s;
	if (!s.create(a.addrN().family()))
		return false;
	const bool used = !s.bind(a);
	s.close();
	return used;
}


/* ExpPortMgr */

ExpPortMgr::ExpPortMgr(const NetAddr &anAddr, int aPortMin, int aPortMax):
	PortMgr(anAddr) {
	theAddr.port(-1);

	if (!TheHistory.configured())
		TheHistory.configure(aPortMin, aPortMax);
}

int ExpPortMgr::allocPort(Socket &s) {
	int port;

	port = findPort(TheHistory.theValidPorts, s);
	if (port < 0)
		port = findPort(TheHistory.theVoidPorts, s);

	if (port < 0) {
		Comment << here << "no ports left at " << theAddr << ";"
			<< " bound: " << BoundLvl().level() 
			<< " valid: " << TheHistory.theValidPorts.count() 
			<< " void:  " << TheHistory.theVoidPorts.count() 
			<< " range: [" << TheHistory.thePortMin << ',' << TheHistory.thePortMax << ']'
			<< endc;
		Assert(TheHistory.theValidPorts.empty());
		Error::Last(EADDRNOTAVAIL);
	} else {
		Assert(TheHistory.belongs(port));
	}
	return port;
}

void ExpPortMgr::freePort(int port, bool good) {
	TheHistory.record(port, good);
}

int ExpPortMgr::findPort(Ring<int> &ports, Socket &s) {
	// note: if ports == theVoidPorts, ports may "grow" inside the loop
	for (int i = ports.count(); i > 0; --i) {
		const int port = ports.dequeue();
		Assert(TheHistory.belongs(port));
		if (!bindToPort(s, port)) {
			if (ReportError2(Error::Last(), lgcCltSide))
				Comment << "failed to bind to " << theAddr << ':' << port << endc;
			TheHistory.record(port, false);
		} else
			return port;
	}
	return -1;
}

bool ExpPortMgr::bindToPort(Socket &s, int port) const {
	NetAddr a(theAddr.addrN(), port);
	return s.bind(a);
}
