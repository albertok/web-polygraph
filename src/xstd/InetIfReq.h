
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_INETIFREQ_H
#define POLYGRAPH__XSTD_INETIFREQ_H

#include "xstd/InAddress.h"

class String;

class InetIfReq {
	typedef char Name[IFNAMSIZ];
	public:
		InetIfReq() { theName[0] = (char)0; }
		InetIfReq(const char *name, InAddress ina): sa(ina) {
			strncpy(theName, name, IFNAMSIZ);
		}
		InetIfReq(const ifreq &ifr): sa(ifr.ifr_addr) {
			strncpy(theName, ifr.ifr_name, IFNAMSIZ);
		}
		const Name &name() const { return theName; }
		const InAddress &addrN() const { return sa; }
	private:
		char theName[IFNAMSIZ];
		InAddress sa;
};

#if !defined(HAVE_TYPE_IFALIASREQ)
struct ifaliasreq {
	char ifra_name[IFNAMSIZ];        // ifname, e.g. "en0"
	sockaddr_in ifra_addr;
	sockaddr_in ifra_broadaddr;
	sockaddr_in ifra_mask;  
};
#endif

class InetIfAliasReq: public ifaliasreq {
	public:
		typedef char Name[IFNAMSIZ];

	public:
		inline InetIfAliasReq(const String &name);

		const Name &name() const { return ifra_name; }
		const InAddress addr() const { return InAddress(ifra_addr); }
		const InAddress broad() const { return InAddress(ifra_broadaddr); }
		const InAddress mask() const { return InAddress(ifra_mask); }

		Name &name() { return ifra_name; }
		void addr(const InAddress &a) { ((sockaddr_in&)ifra_addr) = a.sockAddrFour(0); }
		void broad(const InAddress &a) { ((sockaddr_in&)ifra_broadaddr) = a.sockAddrFour(0); }
		void mask(const InAddress &a) { ((sockaddr_in&)ifra_mask) = a.sockAddrFour(0); }
};

inline
InetIfAliasReq::InetIfAliasReq(const String &aName) {
	memset(this, 0, sizeof(*this));
	strncpy(name(), aName.cstr(), sizeof(Name));
}


#if !defined(HAVE_TYPE_IN6_ALIASREQ)
// Assume we are on Linux if here
#define HAVE_TYPE_LINUX_IN6_IFREQ 1
struct in6_ifreq {
	struct in6_addr ifr6_addr;
	unsigned int ifr6_prefixlen;
	unsigned int ifr6_ifindex;
};
struct in6_aliasreq: public in6_ifreq {
};
#endif

class Inet6IfAliasReq: public in6_aliasreq {
	public:
		Inet6IfAliasReq(const String &name, const InAddress &addr, const InAddress &mask);
};

#endif	/* XSTD_NETWORK_INTERFACE_REQ_CLASS_H */
