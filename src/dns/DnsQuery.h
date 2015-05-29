
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__DNS_DNSQUERY_H
#define POLYGRAPH__DNS_DNSQUERY_H

#include "dns/DnsMsg.h"
#include "xstd/Dns.h"


class Socket;
class OBStream;

// common type for all DNS messages
class DnsQuery: public DnsMsg {
	public:
		DnsQuery(Types aType): theType(aType) {}

		bool sendTo(const NetAddr &ns, Socket &s);

	protected:
		bool dumpAQuery(OBStream &os) const;
		bool dumpName(OBStream &os, const NetAddr &addr) const;

	private:
		Types theType;
};

#endif
