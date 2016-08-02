
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__DNS_DNSMGR_H
#define POLYGRAPH__DNS_DNSMGR_H

#include "xstd/Array.h"
#include "xstd/Socket.h"
#include "xstd/Dns.h"
#include "xstd/FileScanner.h"
#include "dns/DnsMsg.h"

class Client;
class CltXact; // DnsMgr treats this as void
class DnsXact;
class DnsResp;
class DnsResolverSym;
class DnsCache;

// handles asynchronous DNS lookups, with an optional caching capability
class DnsMgr: protected Dns, public FileScanUser {
	public:
		DnsMgr(Client *anOwner);
		virtual ~DnsMgr();

		void configure(const DnsResolverSym *cfg);
		void start();
		void stop();

		typedef enum { dnsAlreadyAnIp, dnsCacheHit, dnsNeedsAsyncLookup } InstantLookupResult;
		InstantLookupResult instantLookup(const NetAddr &addr, NetAddr &ip) const;

		bool lookup(const NetAddr &addr, CltXact *x);

		void noteReadReady(int fd);
		void noteXactDone(DnsXact *x);

		int logCat() const;

		// for DnsXact
		const NetAddr &addr() const;
		Socket &socket() { return theSock; }

	protected:
		void openSocket();
		void noteReply(const DnsResp &m);

	protected:
		Client *theOwner;
		Array<NetAddr*> theServers; // DNS server addresses
		Time theTimeout;            // one for all xactions
		DnsCache *theCache; // optional lookup cache

		Array<DnsXact*> theXacts;   // pending transactions

		Socket theSock;
		FileScanReserv theReserv;
		bool closeWithLast;
		DnsMsg::Types theType;
};

#endif
