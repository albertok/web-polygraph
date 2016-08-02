
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLNETADDRRANGE_H
#define POLYGRAPH__PGL_PGLNETADDRRANGE_H

#include "pgl/PglStrRange.h"

class NetAddr;
class NetAddrSym;

class PglNetAddrRange: public PglStrRange {
	public:
		typedef void (*AddrIter)(const NetAddr &);

	public:
		PglNetAddrRange();

		virtual void reset();

		// use subnet specs or guess an approprite netmask for the range
		void netmask(struct InAddress &pin) const;

		virtual ArraySym *toSyms(const TokenLoc &loc) const;
		void toAddrs(Array<NetAddr*> &addrs) const;
		void toAddrs(AddrIter iter) const;
		void addrAt(int idx, NetAddrSym &nas) const;

		virtual bool parse(const String &val);
		virtual ostream &print(ostream &os) const;

	protected:
		virtual bool canMergeSameType(const StringArrayBlock &b) const;
		virtual void mergeSameType(const StringArrayBlock &b);
		virtual bool optRangeBeg(char ch) const;

		void currentAddrSym(NetAddrSym &nas) const;
		void currentNetAddr(NetAddr &addr) const;

	protected:
		String theIfName;   // interface name
		String theAddr;     // address:port
		int theSubnet;      // subnet specs
};

#endif
