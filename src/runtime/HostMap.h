
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_HOSTMAP_H
#define POLYGRAPH__RUNTIME_HOSTMAP_H

#include "xstd/Array.h"
#include "xstd/NetAddr.h"
#include "runtime/httpHdrs.h"

class ContentSel;
class ServerRep;
class SslWrap;
class ObjUniverse;
class HttpCookies;
class HostsBasedSym;

// Host configuration record
class HostCfg {
	public:
		HostCfg(const NetAddr &anAddr);

	public:
		const NetAddr theAddr;
		Agent::Protocol theProtocol;
		ContentSel *theContent;    // can be shared among HostCfgs
		ServerRep *theServerRep;
		ObjUniverse *theUniverse; // for visible servers only
		const HostsBasedSym *theHostsBasedCfg; // Proxy for SSL-to-proxy
		bool isSslActive; // whether SSL is configured and supported
};

// a NetAddr <-> Host configuration map
// add-only operation
class HostMap {
	public:
		HostMap(int aCapacity); // cap may be adjusted a bit
		~HostMap();

		int hostCount() const { return theCount; }
		int iterationCount() const { return theStaticIndex.count() + theDynamicIndex.count(); }

		HostCfg *at(int idx) const; // idx may be out of bounds
		HostCfg *at(const NetAddr &addr);

		ServerRep *serverRepAt(int idx);
		bool sslActive(const NetAddr &addr);
		ObjUniverse *findUniverse(const NetAddr &addr);
		ObjUniverse *findUniverseAt(int idx);

		HostCfg *find(const NetAddr &addr);
		HostCfg *find(const NetAddr &addr, int &idx);
		bool findIdx(const NetAddr &addr, int &idx);
		HostCfg *addAt(int idx, const NetAddr &addr);

	protected:
		int capacity() const { return theStaticIndex.capacity(); }
		bool findIdxInIndex(const Array<HostCfg*> &arr, const NetAddr &addr, int &idx) const;
		bool endSearch(const Array<HostCfg*> &arr, const NetAddr &addr, int idx, bool &res) const;
		int hash0(const NetAddr &addr) const;
		int hash1(const NetAddr &addr) const;

	protected:
		Array<HostCfg*> theStaticIndex; // non-dynamic host addresses
		Array<HostCfg*> theDynamicIndex; // dynamic host addresses
		int theCount;  // entries in both hashes
};

extern HostMap *TheHostMap; // move to globals?

#endif
