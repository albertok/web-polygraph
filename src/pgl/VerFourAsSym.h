
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_VERFOURASSYM_H
#define POLYGRAPH__PGL_VERFOURASSYM_H

#include "pgl/AddrSchemeSym.h"

class NetAddr;
class BenchSideSym;
class PglNetAddrRange;
class ArraySym;
class NetAddrSym;

// common class for all address schemes in version -4 workloads
class VerFourAsSym: public AddrSchemeSym {
	public:
		static const String TheType;

	public:
		VerFourAsSym(const String &aType, PglRec *aRec);

		bool agentsPerAddr(int &apa) const;

	protected:
		String addresses(const BenchSideSym *side, ArraySym *&addrs) const;
		String distributeAddrs(ArraySym *&addrs, const ArraySym *addrSpace, int hostCnt, int agentCnt, int addrLoad) const;
		String skipSubnet(const NetAddr &lastAddr, int subnet, const ArraySym *addrSpace, int &spaceIdx) const;
		bool sameSubnet(const NetAddr &addr1, const NetAddr &addr2, int subnet) const;
		int addAddr(Array<PglNetAddrRange*> &ranges, const NetAddrSym &addr, int subnet) const;
};

#endif
