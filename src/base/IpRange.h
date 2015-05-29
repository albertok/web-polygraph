
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_IPRANGE_H
#define POLYGRAPH__BASE_IPRANGE_H

#include "xstd/NetAddr.h"


template <class>
class Array;


// Stores and manipulates IP ranges like a-b.c-d.e.f
class IpRange {
public:

	IpRange() {};
	IpRange(const NetAddr &a);
	bool merge(const IpRange &r);
	bool includes(const IpRange &r) const;
	int count() const;
	ostream &print(ostream &os) const;

private:

	bool canMerge(const IpRange &r) const;
	int addrPart(const int idx) const;
	int partLimit() const { return theAddr.addrN().family() == AF_INET6 ? 8 : 4; }

	NetAddr theAddr; // start addr
	int theRanges[8]; // range length for each address part
};

inline ostream &operator <<(ostream &os, const IpRange &range) { return range.print(os); }

// compacts all addresses or ranges to rages
extern void CompactAllAddrs(const Array<NetAddr> &in, Array<IpRange> &out);

#endif
