
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/net/if.h"
#include "xstd/h/netinet.h"
#include "xstd/h/iostream.h"

#include "xstd/Assert.h"
#include "xstd/getIfAddrs.h"
#include "xstd/gadgets.h"
#include "xstd/InetIfReq.h"

#if HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

#if WIN32 /* XXX: merge (declare our WIN32 specific ifconf) */
bool GetIfAddrs(Array<InetIfReq> &addrs, const String &ifname, Array<InAddress> *netmasks) {
	if (!ifname)
		ifname = "any";

	Socket s;
	if (!s.create(PF_INET, SOCK_DGRAM, 0))
		return false;

	INTERFACE_INFO InterfaceList[20];
	unsigned long nBytesReturned;
	if (WSAIoctl(s.fd(), SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
		sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR)
		return false;

	const int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);
	for (int i = 0; i < nNumInterfaces; ++i) {
		ifreq req;
		strncpy(req.ifr_name, ifname, sizeof(req.ifr_name));
		memcpy(&req.ifr_addr, &InterfaceList[i].iiAddress, sizeof(req.ifr_addr));
		addrs.append(req);
		if (netmasks)
			netmasks->append(InAddress()); // TODO: How to get the netmask?
	}

	s.close();
	return true;
}
#else

struct ifreq;


class IfConf: public ifconf {
	public:
		IfConf(int len = 0) { ifc_buf = 0; setLen(len); }
		~IfConf() { setLen(0); }

		void setLen(int len);

		int reqSize(const struct ifreq *req) const;
		const ifreq *req(int off) const { return (const ifreq *)(ifc_buf + off); }
		operator void*() const { return (void*)ifc_buf; }
};

void IfConf::setLen(int len) { 
	delete[] ifc_buf;
	ifc_buf = len > 0 ? new char[len] : 0;
	ifc_len = len;
}

int IfConf::reqSize(const struct ifreq *req) const {
#ifndef WIN32
	// note: there are many broken cases not covered here;
	// see what /usr/src/contrib/bind/bin/named/ns_main.c has to do
#	if defined(HAVE_SA_LEN)
		const int varLen = Max((int)sizeof(sockaddr), (int)req->ifr_addr.sa_len);
#	else
		const int varLen = (int)sizeof(sockaddr);
#	endif
	return sizeof(req->ifr_name) + varLen;
#else /* WIN32 */
	return sizeof(*req);
#endif
}

static
bool ifNameMatch(const String &ifname, const String &aname) {
	if (const char *p = aname.chr(':'))
		return ifname.cmp(aname.data(), p - aname.data()) == 0;
	else
		return aname == ifname;
}

#if HAVE_GETIFADDRS
bool GetIfAddrs(Array<InetIfReq> &addrs, const String &ifname, Array<InAddress> *netmasks) {
	struct ifaddrs *ifap = 0;
	struct ifaddrs *ifp = 0;
	if (getifaddrs(&ifap)) {
		// Error() ?
		return false;
	}
	for (ifp = ifap; ifp; ifp = ifp->ifa_next) {
		if (!ifp->ifa_addr)
			continue;
		if (ifname && !ifNameMatch(ifname, ifp->ifa_name))
			continue;
		if (AF_INET == ifp->ifa_addr->sa_family ||
			AF_INET6 == ifp->ifa_addr->sa_family) {
			InetIfReq req(ifp->ifa_name, InAddress(*ifp->ifa_addr));
			addrs.push(req);
			ifp->ifa_netmask->sa_family = ifp->ifa_addr->sa_family;
			if (netmasks)
				netmasks->push(InAddress(*ifp->ifa_netmask));
		}
	}
	freeifaddrs(ifap);
	return true;
}

#else

static int ifreqSize(const ifreq *req);

// guess how big the cfg buffer should be
// should this be a Socket method?
static
bool GuessIfConfSize(Socket s, IfConf &cfg) {
	int last_returned_len = -1;
	// grow the buffer until we get errors or the answer fits
	// the asnwer fits if returned len does not change
	for (int buf_len = 4*1024; s; buf_len *= 2) {
		cfg.setLen(buf_len);
		if (!s.getIfConf(cfg))
			return false;
		if (cfg.ifc_len == last_returned_len)
			return true;
		last_returned_len = cfg.ifc_len;
	}
	return false;
}

// Get a list of all interface addresses using Socket::getIfConf.  
// WARNING: this is IPv4 only!
//
bool GetIfAddrs(Array<InetIfReq> &addrs, const String &ifname, Array<InAddress> *netmasks) {
	Socket s;
	if (!s.create(PF_INET, SOCK_DGRAM, 0))
		return false;

	IfConf cfg;
	if (GuessIfConfSize(s, cfg)) {
		const ifreq *req;
		for (int off = 0; off < cfg.ifc_len; off += ifreqSize(req)) {
			req = cfg.req(off);

			if (req->ifr_addr.sa_family != AF_INET)
				continue;

			if (ifname && !ifNameMatch(ifname, req->ifr_name))
				continue;

			addrs.append(*req);
			if (netmasks)
				netmasks->append(InAddress()); // TODO: How to get the netmask?
		}
	}

	s.close();
	return true;
}


// calculate the actual size of ifreq structure
static
int ifreqSize(const ifreq *req) {
	// note: there are many broken cases not covered here;
	// see what /usr/src/contrib/bind/bin/named/ns_main.c has to do

#if defined HAVE_SA_LEN
	return Max(sizeof(*req), req->ifr_addr.sa_len + sizeof(req->ifr_name));
#else
	return sizeof(*req);
#endif
}
#endif /* else HAVE_GETIFADDRS */

#endif /* WIN32 */
