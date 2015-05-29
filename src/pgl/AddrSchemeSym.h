
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_ADDRSCHEMESYM_H
#define POLYGRAPH__PGL_ADDRSCHEMESYM_H

#include "pgl/PglRecSym.h"
class NetAddr;
class NetAddrSym;
class PglNetAddrRange;
class ArraySym;
class BenchSym;

class AddrSchemeSym: public RecSym {
	public:
		static String TheType;

	public:
		typedef ArraySym *(AddrSchemeSym::*AddrCalc)(const BenchSym *bench, String &err) const;

	public:
		AddrSchemeSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		String kind() const;

		ArraySym *robots(const BenchSym *bench, String &err) const;
		ArraySym *servers(const BenchSym *bench, String &err) const;
		ArraySym *proxies(const BenchSym *bench, String &err) const;

	protected:
		virtual String robots(ArraySym *&addrs) const = 0;
		virtual String servers(ArraySym *&addrs) const = 0;
		virtual String proxies(ArraySym *&addrs) const;

		void makeAddrSym(const NetAddrSym &mask, int x, int y, int subnet, NetAddrSym &nas) const;

		//ArraySym *ipStrToArr(const String &str) const;
		//ArraySym *addresses(const NetAddr &mask, double reqRate, double hostLoad, double agentLoad, int xOffset) const;

		String minSubnet(int maxAddrPerHost, int &minSubnet) const;
		String ipRangeToStr(const NetAddrSym &mask, int maxX, int countX, int minY, int countY) const;
		int singleDiv(int factor, double n) const;
		int doubleDiv(int factor, double n, double d) const;
		void printOctetRange(ostream &os, int beg, int count) const;

		static void addAddrToRanges(Array<PglNetAddrRange*> &ranges, const NetAddrSym &addr);
		static void mergeRanges(Array<PglNetAddrRange*> &ranges);
		static ArraySym *rangesToAddrs(const Array<PglNetAddrRange*> &ranges, const int addrLoad);

		virtual void setBench(const BenchSym *bench) const;

		void kind(const String &aKind);

	protected:
		mutable const BenchSym *theBench; // valid during robots/servers calls
};

#endif
