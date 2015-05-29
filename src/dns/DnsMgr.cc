
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include "xstd/h/iomanip.h"

#include "xstd/NetAddr.h"
#include "runtime/Farm.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "runtime/globals.h"
#include "pgl/DnsResolverSym.h"
#include "dns/DnsQuery.h"
#include "dns/DnsResp.h"
#include "dns/DnsXact.h"
#include "dns/DnsMgr.h"
#include "client/Client.h"


static ObjFarm<DnsXact> TheXacts;


DnsMgr::DnsMgr(Client *anOwner): theOwner(anOwner), closeWithLast(false), theType(DnsMsg::typeA) {
	Assert(theOwner);
}

DnsMgr::~DnsMgr() {
	if (theReserv)
		TheFileScanner->clearRes(theReserv);
	if (theSock)
		theSock.close();
}

void DnsMgr::configure(const DnsResolverSym *cfg) {
	Assert(cfg);

	cfg->servers(theServers); 
	// set default DNServer port
	for (int i = 0; i < theServers.count(); ++i) {
		if (theServers[i]->port() < 0)
			theServers[i]->port(53);
	}

	theTimeout = cfg->timeout();
	if (theServers.count() && theTimeout < 0) {
		static const void *lastCfg = 0;
		if (lastCfg != cfg) {
			cerr << cfg->loc() << "warning: no timeout for DNS queries specified" << endl;
			lastCfg = cfg;
		}
	}

	if (const String s = cfg->queryType()) {
		if (0 == s.cmp("AAAA"))
			theType = DnsMsg::typeAAAA;
		else
			cerr << here << "unknown query type: " << s
			<< endl << xexit;
	}

	TheXacts.limit(1024); // magic, no good way to estimate
}

const NetAddr &DnsMgr::addr() const {
	Assert(theOwner);
	return theOwner->host();
}

void DnsMgr::start() {
	closeWithLast = false;
}

void DnsMgr::stop() {
	clearCache();

	closeWithLast = true;
	if (!theXacts.count() && theSock)
		theSock.close();
}

void DnsMgr::clearCache() {
	// no cache yet
}

void DnsMgr::openSocket() {
	if (Should(theSock.create(theOwner->host().addrN().family(), SOCK_DGRAM, 0))) {
		Should(theSock.blocking(false));
		Should(theSock.reuseAddr(true));
		if (theOwner->host())
			Should(theSock.bind(NetAddr(theOwner->host().addrN(), 0)));
	}
}

bool DnsMgr::needsLookup(const NetAddr &addr) const {
	// all domain names require lookup -- cache is not yet implemeneted
	return addr.isDomainName();
}

// returns false if lookup is not possible
bool DnsMgr::lookup(const NetAddr &addr, CltXact *reason) {
	if (!theServers.count())
		cerr << here << "the DNS resolver was not configured "
			<< " (no DNS servers were specified); cannot resolve " << addr 
			<< endl << xexit;

	DnsXact *x = TheXacts.get();
	x->reason(reason);
	x->queryAddr(addr);
	x->timeout(theTimeout);
	x->type(theType);

	x->idx(theXacts.count());
	theXacts.append(x);

	/* make sure we are ready for I/O */

	if (!theSock) {
		openSocket();
		if (!theSock)
			return false;
	}
	
	if (!theReserv)
		theReserv = TheFileScanner->setFD(theSock.fd(), dirRead, this);

	x->exec(this, *theServers[0]);
	return true;
}

void DnsMgr::noteReadReady(int fd) {
	Assert(fd == theSock.fd());
	DnsResp m;
	while (m.recv(theSock)) {
		noteReply(m); // note: xactions will call back
		m.reset();
	}
}

void DnsMgr::noteReply(const DnsResp &m) {
	// find the corresponding transaction
	// XXX: slow, replace theXacts with a id-based hash
	for (int i = 0; i < theXacts.count(); ++i) {
		if (DnsXact *x = theXacts[i]) {
			if (x->id() == m.id()) {
				Assert(x->idx() == i);
				x->noteReply(m);
				return;
			}
		}
	}

	/* not found */

	if (DnsMsg::LastId() < m.id())
		ReportError(errDnsBadMsgId);
	else
	if (m.error())
		ReportError(m.error());

	// else probably timedout
}

void DnsMgr::noteXactDone(DnsXact *x) {
	Assert(x);

	const NetAddr addr = x->respAddr();
	CltXact *reason = x->reason();

	if (x->needRetry()) {
		if (x->tryCount() < theServers.count()) {
			x->retry(*theServers[x->tryCount()]);
			return;
		}
		if (theServers.count() > 1)
			ReportError(errDnsAllSrvsFailed);
	}

	const int idx = x->idx();
	Assert(theXacts[idx] == x);
	// remove xaction from the index, put last xaction in its place
	// a no-op if x is last
	DnsXact *last = theXacts.last();
	theXacts[idx] = last;
	last->idx(idx);
	theXacts.pop();

	TheXacts.put(x);

	if (!theXacts.count()) {
		if (theReserv)
			TheFileScanner->clearRes(theReserv);
		if (closeWithLast)
			theSock.close();
	}

	theOwner->noteAddrLookup(addr, reason);
}

int DnsMgr::logCat() const {
	Assert(theOwner);
	return theOwner->logCat();
}
