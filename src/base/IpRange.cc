
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Array.h"

#include "base/IpRange.h"


IpRange::IpRange(const NetAddr &a):
	theAddr(a) {
	for (int i = 0; i < partLimit(); ++i)
		theRanges[i] = 1;
}

// only consecutive ranges that differ in one octet can be merged
// e.g. 127.0.1-2.3-4 and 127.0.3-4.3-4 can be merged
//      127.0.2-3.3-4 and 127.0.0-1.3-4 can not
// this is enough since the purpose is to merge back ranges generated
// by standard address schemes
bool IpRange::canMerge(const IpRange &r) const {
	if (theAddr.addrN().family() != r.theAddr.addrN().family())
		return false;

	int diffs(0);
	for (int i = 0; i < partLimit() && diffs <= 1; ++i) {
		if (addrPart(i) + theRanges[i] == r.addrPart(i))
			++diffs;
		else
		if (addrPart(i) != r.addrPart(i) ||
			theRanges[i] != r.theRanges[i])
			return false;
	}
	return diffs == 1;
}

// merge is done in one octet
// if ranges differ in more than one octet they can not be merged
bool IpRange::merge(const IpRange &r) {
	if (canMerge(r)) {
		for (int i = 0; i < partLimit(); ++i) {
			if (addrPart(i) != r.addrPart(i)) {
				theRanges[i] += r.theRanges[i];
				return true;
			}
		}
		Assert(false);
	}
	return false;
}

int IpRange::addrPart(const int idx) const {
	Assert(0 <= idx && idx <= partLimit());
	return theAddr.addrN().family() == AF_INET6 ?
		(theAddr.octet(idx * 2) << 8) + theAddr.octet(idx * 2 + 1) :
		theAddr.octet(idx);
}

bool IpRange::includes(const IpRange &r) const {
	if (theAddr.addrN().family() != r.theAddr.addrN().family())
		return false;

	for (int i = 0; i < partLimit(); ++i) {
		if (r.addrPart(i) < addrPart(i) ||
			addrPart(i) + theRanges[i] < r.addrPart(i) + r.theRanges[i])
			return false;
	}

	return true;
}

int IpRange::count() const {
	int count = 1;
	for (int i = 0; i < partLimit(); ++i)
		count *= theRanges[i];
	return count;
}

ostream &IpRange::print(ostream &os) const {
	if (theAddr.addrN().family() == AF_INET6)
		os << '[' << hex;
	for (int i = 0; i < partLimit(); ++i) {
		if (i)
			os << (theAddr.addrN().family() == AF_INET6 ? ':' : '.');
		os << addrPart(i);
		if (theRanges[i] > 1)
			os << '-' << (addrPart(i) + theRanges[i] - 1);
	}
	if (theAddr.addrN().family() == AF_INET6)
		os << dec << ']';
	return os;
}


// single pass merge, i.e. merges ranges in one octet
// the octet to be merged is the first mergeable octet found
// run multiple times to merge ranges in more than one octet
// template lets us use both NetAddr and IpRange arrays as input
template<class T>
static bool CompactAddrs(const Array<T> &in, Array<IpRange> &out) {
	if (in.empty())
		return true;

	out.reset();
	out.stretch(in.count());
	IpRange r(in[0]);
	for (int i = 1; i < in.count(); ++i) {
		// the mergeable octet position may change from
		// one r.merge call to the next
		if (!r.merge(in[i])) {
			out.append(r);
			r = in[i];
		}
	}
	out.append(r);
	return out.count() == in.count();
}

void CompactAllAddrs(const Array<NetAddr> &in, Array<IpRange> &out) {
	Array<IpRange> *result = new Array<IpRange>;
	bool stop = CompactAddrs(in, *result);
	while (!stop) {
		const Array<IpRange> *const i = result;
		result = new Array<IpRange>;
		stop = CompactAddrs(*i, *result);
		delete i;
	}
	out = *result;
	delete result;
}
