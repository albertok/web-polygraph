
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_ADDRSUBSTS_H
#define POLYGRAPH__RUNTIME_ADDRSUBSTS_H

#include "xstd/Array.h"
#include "xstd/NetAddr.h"

class AddrSubsts;
class ContainerSym;


// addr iterator for the map below
class AddrSubstsIter {
	public:
		AddrSubstsIter(const AddrSubsts &aSubs, const NetAddr &orig);

		operator void *() const { return atEnd() ? 0 : (void*)-1; }
		AddrSubstsIter &operator ++() { ++theAddrIdx; sync(); return *this; }

		const NetAddr &addr();

	protected:
		bool atEnd() const;
		void sync();

	protected:
		const AddrSubsts &theSubs;
		NetAddr theOrig;
		int theGroupIdx;
		int theAddrIdx;
		bool checkMemb;
};

// one substitute group (addresses than can substitute each other)
class AddrSubstGroup: public Array<NetAddr*> {
	public:
		typedef Array<NetAddr*> Addrs;
		
	public:
		AddrSubstGroup(int aCap);
		void configure(const ContainerSym &group);

		bool member(const NetAddr &addr) const;
};

// all substitute groups
class AddrSubsts {
	public:
		friend class AddrSubstsIter;
		typedef AddrSubstGroup Group;
		typedef Array<Group*> Groups;
		typedef AddrSubstsIter Iterator;

	public:
		AddrSubsts();
		~AddrSubsts();

		void configure(const Array<ContainerSym*> &groups);

		int count() const { return theCount; }

		Iterator iterator(const NetAddr &orig) const;

	protected:
		const NetAddr &at(int group, int idx) const;

	protected:
		Groups theGroups;
		int theCount; // all addresses in all groups
};

extern AddrSubsts *TheAddrSubsts;

#endif
