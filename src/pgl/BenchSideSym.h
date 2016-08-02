
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_BENCHSIDESYM_H
#define POLYGRAPH__PGL_BENCHSIDESYM_H

#include "xstd/Array.h"
#include "xstd/NetAddr.h"
#include "pgl/PglRecSym.h"

class String;
class IntSym;
class ArraySym;
class NetAddrSym;
class BenchSym;

// bench side configuration information (as in "client side" of the bench)
class BenchSideSym: public RecSym {
	public:
		static String TheType;

	public:
		BenchSideSym();
		BenchSideSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		const BenchSym *bench() const { return theBench; }
		void bench(const BenchSym *aBench) const { theBench = aBench; }

		bool hosts(Array<NetAddr *> &addrs) const;
		bool cpuCoresArray(Array< Array<int> > &cpuCores) const;
		ArraySym *hostsSym() const;
		const ArraySym *addrSpace() const;
		NetAddr addrMask() const;
		NetAddrSym* addrMaskSym() const;
		bool maxHostLoad(double &load) const;
		bool maxAgentLoad(double &load) const;
		bool addresses(Array<NetAddrSym *> &addrs) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

	protected:
		mutable const BenchSym *theBench; // set by the bench
};

#endif
