
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <ctype.h>
#include "xstd/h/iostream.h"

#include "xstd/Assert.h"
#include "xstd/NetAddr.h"
#include "xstd/gadgets.h"


NetAddr::NetAddr(const struct sockaddr_storage &ss): theAddrN(ss),
	thePort(InAddress::GetPort(ss)) {
}

// a.b.c.d = 0.1.2.3
int NetAddr::octet(int idx) const {
	return addrN().octet(idx);
}

// a.b.c.d = 3.2.1.0
int NetAddr::roctet(int idx) const {
	return addrN().roctet(idx);
}

void NetAddr::syncA() const {
	theAddrA = theAddrN.known() ? theAddrN.image() : 0;
}

void NetAddr::syncN() const {
	if (theAddrA && !isDomainName())
		theAddrN.pton(theAddrA.cstr());
	else
		theAddrN.known(false);
}

bool NetAddr::operator == (const NetAddr &addr) const {
	if (thePort != addr.thePort)
		return false;
	return sameButPort(addr);
}

bool NetAddr::sameButPort(const NetAddr &addr) const {
	if (theAddrN.known() && addr.theAddrN.known())
		return theAddrN == addr.theAddrN;

	return addrA() == addr.addrA();
}

int NetAddr::compare(const NetAddr &addr) const {
	if (thePort != addr.thePort)
		return thePort > addr.thePort ? +1 : -1;

	if (addrN().known() && addr.addrN().known())
		return addrN().compare(addr.addrN());

	return addrA().cmp(addr.addrA());
}

bool NetAddr::isDomainName() const {
	// domain name can not be an IP address, use theAddrA directly
	if (!theAddrA)
		return false;

	const char first = *theAddrA;
	if (first == '[') // IPv6 images start with a square bracket
		return false;

	if (!isdigit(first)) // IPv4 never starts with a non-digit
		return true;

	// domain name must end with a non-digit (but .last() is slower than first)
	return !isdigit(theAddrA.last());
}

bool NetAddr::isDynamicName() const {
	// dynamic name can not be an IP address, use theAddrA directly
	return theAddrA && *theAddrA == '.';
}

bool NetAddr::suffixMatches(const NetAddr &addr) const {
	Assert(isDynamicName());
	if (addr.addrA().endsWith(theAddrA.data(),
		theAddrA.len()) &&
		thePort == addr.thePort)
		return true;
	return false;
}

ostream &NetAddr::print(ostream &os) const {
	if (addrA())
		os << theAddrA;
	else
	if (thePort >= 0)
		os << '*';
	else
		os << "<none>";

	if (thePort >= 0)
		os << ':' << thePort;

	return os;
}


#if !defined(HAVE_INET_MAKEADDR) && defined(IN_CLASSA_HOST)

/* Formulate an Internet address from network + host. Used in
 * building addresses stored in the ifnet structure.
 * (c) The Regents of the University of California */
in_addr inet_makeaddr(unsigned long net, unsigned long host) {
	unsigned long addr;
	if (net < 128)
		addr = (net << IN_CLASSA_NSHIFT) | (host & IN_CLASSA_HOST);
	else 
	if (net < 65536)
		addr = (net << IN_CLASSB_NSHIFT) | (host & IN_CLASSB_HOST);
	else
	if (net < 16777216L)
		addr = (net << IN_CLASSC_NSHIFT) | (host & IN_CLASSC_HOST);
	else
		addr = net | host;
	addr = htonl(addr);
	return (in_addr&)addr;
}

unsigned long inet_lnaof(in_addr in) {
	const unsigned long i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return (i & IN_CLASSA_HOST);
	else
	if (IN_CLASSB(i))
		return (i & IN_CLASSB_HOST);
	else
		return (i & IN_CLASSC_HOST);

}

unsigned long inet_netof(in_addr in) {
	const unsigned long i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return ((i & IN_CLASSA_NET) >> IN_CLASSA_NSHIFT);
	else
	if (IN_CLASSB(i))
		return ((i & IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);
	else
		return ((i & IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);
}

#endif

struct sockaddr_storage NetAddr::sockAddr() const {
	return addrN().sockAddr(thePort);
}

unsigned int NetAddr::hash0() const {
	if (addrN().known())
		return addrN().hash0() + thePort;
	else
		return addrA().hash() + thePort;
}

unsigned int NetAddr::hash1() const {
	if (addrN().known())
		return (addrN().hash0() ^ thePort) + addrN().hash1();
	else
		return (addrA().hash() ^ thePort) - thePort;
}
