
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_SPREADASSYM_H
#define POLYGRAPH__PGL_SPREADASSYM_H

#include "pgl/AddrSchemeSym.h"

class BenchSideSym;

// Spread addressing scheme
class SpreadAsSym: public AddrSchemeSym {
	public:
		static const String TheType;

	public:
		SpreadAsSym();
		SpreadAsSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool agentsPerAddr(int &apa) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		virtual String robots(ArraySym *&addrs) const;
		virtual String servers(ArraySym *&addrs) const;

		String addresses(const BenchSideSym *side, ArraySym *&addrs) const;
		void distributeAddrs(ArraySym *&addrs, const ArraySym *addrSpace, int hostCnt, int agentCnt, int addrLoad) const;
};

#endif
