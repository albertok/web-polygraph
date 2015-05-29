
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_ADDRMAPITEMS_H
#define POLYGRAPH__RUNTIME_ADDRMAPITEMS_H

#include "xstd/Array.h"
#include "xstd/NetAddr.h"

class NetAddrSym;

// base class for all address map items
class AddrMapItem {
	public:
		AddrMapItem(const NetAddr &aName);
		AddrMapItem(const NetAddrSym &aName);
		virtual ~AddrMapItem() {}

		virtual int addrCount() const = 0;
		virtual const NetAddr &addrAt(int addrIdx) const = 0;
		const NetAddr &name() const { return theName; }

		virtual const NetAddr &selectAddr() const = 0;
		virtual double newProb() const { return theNewProb; }

	protected:
		NetAddr theName;
		double theNewProb;
};

// name->name mapping (useful for "no-op" maps)
class Name2NameMapItem: public AddrMapItem {
	public:
		Name2NameMapItem(const NetAddr &aName);

		virtual int addrCount() const { return 1; }
		virtual const NetAddr &addrAt(int) const { return name(); }
		virtual const NetAddr &selectAddr() const { return name(); }
};

// name->addr mapping (useful for simple 1:1 maps)
class Name2AddrMapItem: public AddrMapItem {
	public:
		Name2AddrMapItem(const NetAddrSym &aName, const NetAddr &addr);

		virtual int addrCount() const { return 1; }
		virtual const NetAddr &addrAt(int) const { return theAddr; }
		virtual const NetAddr &selectAddr() const { return theAddr; }

	protected:
		NetAddr theAddr;
};

// name->addrs mapping (useful for 1:N maps)
class Name2AddrsMapItem: public AddrMapItem {
	public:
		Name2AddrsMapItem(const NetAddrSym &aName, const Array<NetAddr*> &anAddrs);
		virtual ~Name2AddrsMapItem();

		virtual int addrCount() const { return theAddrs.count(); }
		virtual const NetAddr &addrAt(int addrIdx) const { return *theAddrs[addrIdx]; }
		virtual const NetAddr &selectAddr() const;

	protected:
		Array<NetAddr*> theAddrs;
};



#endif
