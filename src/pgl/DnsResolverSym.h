
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_DNSRESOLVERSYM_H
#define POLYGRAPH__PGL_DNSRESOLVERSYM_H

#include "xstd/Time.h"
#include "xstd/Array.h"
#include "pgl/PglRecSym.h"

class NetAddr;
class DnsCacheSym;

// parameters for object life cycle model
class DnsResolverSym: public RecSym {
	public:
		static String TheType;

	public:
		DnsResolverSym();
		DnsResolverSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool servers(Array<NetAddr*> &srvs) const;
		Time timeout() const;
		String queryType() const;
		DnsCacheSym *cache() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
