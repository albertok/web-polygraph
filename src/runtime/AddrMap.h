
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_ADDRMAP_H
#define POLYGRAPH__RUNTIME_ADDRMAP_H

#include "xstd/Array.h"

class ArraySym;
class NetAddr;
class NetAddrSym;
class AddrMapSym;
class AddrMap;
class AddrMapItem;

// addr iterator for the map below
class AddrMapAddrIter {
	public:
		AddrMapAddrIter(const AddrMap &aMap, int aName);

		operator void *() const { return atEnd() ? 0 : (void*)-1; }
		AddrMapAddrIter &operator ++() { next(); return *this; }

		const NetAddr &addr();
		const NetAddr &name();

	protected:
		bool atEnd() const;
		void next() { ++theAddr; }

	protected:
		const AddrMap &theMap;
		int theName;
		int theAddr;
};

// name -> address map
// note that find() has to optimize the map if items were added
// so calling add/query often run-time is not a good idea
class AddrMap {
	public:
		typedef Array<NetAddr*> Addrs;
		
	public:
		AddrMap();
		~AddrMap();

		void configure(const Array<AddrMapSym*> &maps);

		int nameCount() const;
		const NetAddr &nameAt(int nameIdx) const;
		const AddrMapItem &itemAt(int nameIdx) const;

		// use iterator instead
		const NetAddr &addrAt(int nameIdx, int addrIdx) const;
		int addrCountAt(int nameIdx) const;

		bool has(const NetAddr &name) const;
		bool find(const NetAddr &name, int &idx) const;
		void add(const NetAddr &name); // name->name

		bool findAddr(const NetAddr &addr) const;
		const NetAddr &selectAddr(int nameIdx) const;

		AddrMapAddrIter addrIter(int nameIdx) const;
		AddrMapAddrIter addrIter(const NetAddr &name) const;

	protected:
		void config1to1(const NetAddrSym &name, const NetAddr &addr);
		void configRoundRobin(const ArraySym* names, const Addrs &addrs);
		void configure1toN(const NetAddrSym &name, const Addrs &addrs);

	protected:
		Array<AddrMapItem*> theMap;
		mutable bool isSorted;
};

extern AddrMap *TheAddrMap;

#endif
