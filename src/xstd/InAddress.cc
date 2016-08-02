
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/string.h"
#include "xstd/h/sys/socket.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/Assert.h"
#include "xstd/InAddress.h"


InAddress::InAddress(): theFamily(0), isKnown(false) {
}

InAddress::InAddress(const sockaddr_storage &s): theFamily(0), isKnown(false) {
	init(s);
}

InAddress::InAddress(const sockaddr_in &s):
	theIPvFour(s.sin_addr), theFamily(AF_INET), isKnown(true) {
}

InAddress::InAddress(const sockaddr_in6 &s):
	theIPvSix(s.sin6_addr), theFamily(AF_INET6), isKnown(true) {
}

InAddress::InAddress(const sockaddr &s): theFamily(0), isKnown(false) {
	init((const sockaddr_storage &)s);
}

InAddress::InAddress(const in_addr &a):
	theIPvFour(a), theFamily(AF_INET), isKnown(true) {
}

InAddress::InAddress(const in6_addr &a):
	theIPvSix(a), theFamily(AF_INET6), isKnown(true) {
}

void InAddress::init(const sockaddr_storage &s) {
	theFamily = s.ss_family;
	known(true);
	switch (theFamily) {
		case AF_INET:
			theIPvFour = ((const sockaddr_in &)s).sin_addr;
			break;
		case AF_INET6:
			theIPvSix = ((const sockaddr_in6 &)s).sin6_addr;
			break;
		case 0:
			known(false);
			break;
		default:
			Should(false);
			known(false);
			break;
	}
}

const unsigned char *InAddress::rawOctets() const {
	return const_cast<InAddress *>(this)->rawOctets();
}

unsigned char *InAddress::rawOctets() {
	switch(theFamily) {
		case AF_INET:
			return reinterpret_cast<unsigned char *>(&theIPvFour);
		case AF_INET6:
			return reinterpret_cast<unsigned char *>(&theIPvSix);
	}
	Assert(!theFamily); // just to distinguish unset from unknown
	Assert(false);
	return 0;
}

// compare() assumes len() does not depend on anything but theFamily
int InAddress::len() const {
	switch(theFamily) {
		case AF_INET: return 4;
		case AF_INET6: return 16;
	}
	return 0;
}

// a.b.c.d = 0.1.2.3
int InAddress::octet(int idx) const {
	Assert(0 <= idx && idx < len());
	return rawOctets()[idx];
}

// octet number couting from reverse/right side
// a.b.c.d = 3.2.1.0
int InAddress::roctet(int idx) const {
	return octet(len()-1-idx);
}

InAddress InAddress::broadcast(const InAddress &netmask) const {
	Assert(theFamily == netmask.theFamily);
	InAddress res(*this);
	unsigned char *ap = res.rawOctets();
	const unsigned char *mp = netmask.rawOctets();
	const int l = len();
	for (int i = 0; i < l; ++i)
		*(ap+i) |= ~(*(mp+i));
	return res;
}

sockaddr_in InAddress::sockAddrFour(int port) const {
	Assert(theFamily == AF_INET);
	sockaddr_in s;
	memset(&s, 0, sizeof(s));
	s.sin_family = theFamily;
	s.sin_addr = theIPvFour;
	s.sin_port = htons(port >= 0 ? port : 0);
#	if HAVE_SA_LEN
		s.sin_len = sizeof(s);
#	endif
	return s;
}

sockaddr_in6 InAddress::sockAddrSix(int port) const {
	Assert(theFamily == AF_INET6);
	sockaddr_in6 s;
	memset(&s, 0, sizeof(s));
	s.sin6_family = theFamily;
	s.sin6_addr = theIPvSix;
	s.sin6_port = htons(port >= 0 ? port : 0);
#	if HAVE_SA_LEN
		s.sin6_len = sizeof(s);
#	endif
	return s;
}

sockaddr_storage InAddress::sockAddr(int port) const {
	sockaddr_storage s;
	memset(&s, 0, sizeof(s));
	switch(s.ss_family = theFamily) {
		case AF_INET: {
			((sockaddr_in&)s) = sockAddrFour(port);
			break;
		}
		case AF_INET6: {
			((sockaddr_in6&)s) = sockAddrSix(port);
			break;
		}
		default: {
			Assert(false);
			break;
		}
	}
	return s;
}

int InAddress::pton(const char *p) {
	int rc = 0;
	if (*p == '[') {
		//
		// XXX ugly memory copy to remove [ ]
		//
		char copy[64];
		strncpy(copy, p+1, 64);
		strtok(copy, "]");
		rc = inet_pton(theFamily = AF_INET6, copy, &theIPvSix);
	} else {
		rc = inet_pton(theFamily = AF_INET, p, &theIPvFour);
	}

	if (rc)
		isKnown = true;
	else
		cerr << here << "inet_pton() can't parse " << p << endl;

	return rc;
}

#define INADDRESS_MAX_NTOP_SZ 64
const char *InAddress::rawImage() const {
	if (known()) {
		static char buf[INADDRESS_MAX_NTOP_SZ];
		inet_ntop(theFamily,
			rawOctets(),
			buf,
			INADDRESS_MAX_NTOP_SZ-1);
		return buf;
	} else {
		return "";
	}
}

const char *InAddress::image() const {
	if (theFamily == AF_INET6) {
		static char buf[INADDRESS_MAX_NTOP_SZ];
		ofixedstream os(buf, sizeof(buf));
		os << '[' << rawImage() << ']' << ends;
		return buf;
	}
	return rawImage();
}
#undef INADDRESS_MAX_NTOP_SZ


unsigned int InAddress::hash0() const {
	unsigned int h = 0;
	int i = len();
	while (i--) {
		h += octet(i);
		h += (h<<10);
		h ^= (h>>6);
	} 
	h += (h<<3);
	h ^= (h>>11);
	h += (h<<15); 
	return h;
}

unsigned int InAddress::hash1() const {
	return roctet(0) + (roctet(1) << 8);
}

bool InAddress::operator == (const InAddress &other) const {
	return compare(other) == 0;
}

int InAddress::compare(const InAddress &other) const {
	if (theFamily != other.theFamily)
		return theFamily > other.theFamily ? +1 : -1;

	return memcmp(rawOctets(), other.rawOctets(), len());
}

InAddress InAddress::NetMask(const int family, int bitCount) {
	InAddress A = family == AF_INET6 ? InAddress::IPvSix() :
		InAddress::IPvFour();
	A.isKnown = true;
	Assert(0 <= bitCount && bitCount <= A.len()*8);
	unsigned char *cp = A.rawOctets();
	memset(cp, 0x00, A.len());
	for (; bitCount > 7; bitCount -= 8)
		*cp++ = 0xff;
	if (bitCount > 0)
		*cp = 0xff << (8 - bitCount);
	return A;
}

bool InAddress::sameSubnet(const InAddress &other, int bitCount) const {
	if (theFamily != other.theFamily)
		return false;
	int sz_bits = len() << 3;
	if (bitCount < 0 || bitCount > sz_bits)
		bitCount = sz_bits;
	const unsigned char *cp1 = rawOctets();
	const unsigned char *cp2 = other.rawOctets();
	for (; bitCount > 7; bitCount -= 8)
		if (*cp1++ != *cp2++)
			return false;
	return (*cp1 >> (8 - bitCount)) == (*cp2 >> (8 - bitCount));
}

int InAddress::prefixlen() const {
	// assume 'rawOctets' is a netmask
	// (lots of ones followed by lots of zeroes)
	int pfx = 0;
	int l = len();
	for (const unsigned char *cp = rawOctets(); l && *cp; l--, cp++) {
		switch (*cp) {
			case 0x80: pfx += 1; break;
			case 0xC0: pfx += 2; break;
			case 0xE0: pfx += 3; break;
			case 0xF0: pfx += 4; break;
			case 0xF8: pfx += 5; break;
			case 0xFC: pfx += 6; break;
			case 0xFE: pfx += 7; break;
			case 0xFF: pfx += 8; break;
			default: Assert(false);
		}
	}
	return pfx;
}


InAddress InAddress::IPvFour() {
	InAddress a;
	a.theFamily = AF_INET;
	a.theIPvFour.s_addr = INADDR_ANY;
	return a;
}

InAddress InAddress::IPvSix() {
	InAddress a;
	a.theFamily = AF_INET6;
	a.theIPvSix = in6addr_any;
	return a;
}

// these two port manipulation methods assume IPv4, but that is safe
// because all sockaddr_* structures have port at the same offset
unsigned short InAddress::GetPort(const sockaddr_storage &s) {
	const struct sockaddr_in &sin = (const struct sockaddr_in&)s;
	return ntohs(sin.sin_port);
}

void InAddress::SetPort(sockaddr_storage &s, int port) {
	struct sockaddr_in &sin = (struct sockaddr_in &)s;
	sin.sin_port = htons(port >= 0 ? port : 0);
}
