
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <unistd.h>
#include <ctype.h>
#include "xstd/h/net/if.h"
#include "xstd/h/netinet.h"
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/gadgets.h"
#include "xstd/getIfAddrs.h"
#include "xstd/InetIfReq.h"
#include "xstd/NetIface.h"

#include <sys/ioctl.h>


bool NetIface::GetAddrs(Array<InetIfReq> &addrs, const String &ifname) {
	return GetIfAddrs(addrs, ifname);
}

NetIface::NetIface() {
	Must(theV4Sock.create(PF_INET, SOCK_DGRAM, 0));
	theV6Sock.create(AF_INET6, SOCK_DGRAM, 0);
}

NetIface::~NetIface() {
	if (theV4Sock)
		theV4Sock.close();
	if (theV6Sock)
		theV6Sock.close();
}

void NetIface::name(const String &aName) {
	theName = aName;
}

bool NetIface::primaries(Primaries &ips) const {
	Assert(theName);

	InAddress v4;
	if (theV4Sock.getV4IfAddr(theName, v4))
		ips.vFour = NetAddr(v4, -1);

	if (theV6Sock) {
		InAddress v6;
		if (theV6Sock.getV6IfAddr(theName, v6))
			ips.vSix = NetAddr(v6, -1);
	}

	return ips.vFour || ips.vSix;
}

int NetIface::nextAliasIndex() const {
	Assert(theName);

	int index = 0;
	Array<InetIfReq> aliases;
	getAliases(aliases);
	if (!aliases.empty()) {
		const char *const p = strchr(aliases.last().name(), ':');
		if (Should(p) && Should(isInt(p + 1, index)))
			++index;
	}
	return index;
}

void NetIface::getAliases(Array<InetIfReq> &aliases) const {
	Assert(theName);
	Must(GetAddrs(aliases, theName));

	Primaries primes;
	if (!primaries(primes))
		return;

	// remove primary address from the aliases array (if any)
	if (primes.vFour)
		ejectPrimary(primes.vFour, aliases);
	if (primes.vSix)
		ejectPrimary(primes.vSix, aliases);
}

void NetIface::ejectPrimary(const NetAddr &primary, Array<InetIfReq> &all) const {
	int paIdx = -1;
	for (int i = 0; paIdx < 0 && i < all.count(); ++i) {
		if (all[i].addrN() == primary.addrN())
			paIdx = i;
	}
	Must(paIdx >= 0);
	if (paIdx < all.count()-1)
		all.memmove(paIdx, all.items()+paIdx+1, all.count()-paIdx-1);
	all.pop();
}

int NetIface::delAliases() {
	Assert(theName);
	Array<InetIfReq> olds;
	getAliases(olds);

	// reverse order seems to be important on Linux where
	// one cannot delete :1 after deleting :0
	int delCount = 0;
	for (int i = olds.count() - 1; i >= 0; --i) {
		const String aliasName = olds[i].name();
		if (delAlias(olds[i].addrN(), aliasName))
			delCount++;
	}
	return delCount;
}

bool NetIface::delAlias(const InAddress &addr, const String &name) {
	Assert(name);
	switch (addr.family()) {
		case AF_INET: {
			InetIfAliasReq r(name);
			r.addr(addr);
			return theV4Sock.delIfAddr(r);
		}
		case AF_INET6: {
			// XXX: implement
			return false;
		}
		default: {
			Error::Last(EINVAL);
			return false;
		}
	}
	Assert(false);
	return false;
}

bool NetIface::addAlias(const InAddress &addr, int idx, const InAddress &netmask) {
	Assert(theName);
	switch (addr.family()) {
		case AF_INET: {
			InetIfAliasReq r(theName);
			r.addr(addr);
			r.mask(netmask);
			r.broad(addr.broadcast(netmask));
			return theV4Sock.addV4IfAddr(r, idx) || !Error::LastExcept(EEXIST);
		}
		case AF_INET6: {
			Inet6IfAliasReq r(theName, addr, netmask);
			return theV6Sock.addV6IfAddr(r, idx) || !Error::LastExcept(EEXIST);
		}
		default: {
			Error::Last(EINVAL);
			return false;
		}
	}
	Assert(false);
	return false;
}

bool NetIface::addAliases(const Array<InAddress> &aliases, const InAddress &netmask) {
	const int offset = nextAliasIndex();
	for (int i = 0; i < aliases.count(); ++i) {
		if (!addAlias(aliases[i], i + offset, netmask))
			return false;
	}
	return true;
}
