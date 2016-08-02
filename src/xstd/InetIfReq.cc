
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/Assert.h"
#include "xstd/String.h"
#include "xstd/Socket.h"
#include "xstd/h/sys/ioctl.h"
#include "xstd/InetIfReq.h"


Inet6IfAliasReq::Inet6IfAliasReq(const String &name, const InAddress &addr, const InAddress &mask) {
	memset(this, 0, sizeof(*this));
#if HAVE_TYPE_IN6_ALIASREQ
	strncpy(ifra_name, name.cstr(), IFNAMSIZ);
	sockaddr_storage ss = addr.sockAddr(-1);
	memcpy(&ifra_addr, &ss, ss.ss_len);
	ss = mask.sockAddr(-1);
	memcpy(&ifra_prefixmask, &ss, ss.ss_len);
	ifra_lifetime.ia6t_expire = 0xffffffff;
	ifra_lifetime.ia6t_preferred = 0xffffffff;
	ifra_lifetime.ia6t_vltime = 0xffffffff;
	ifra_lifetime.ia6t_pltime = 0xffffffff;
#elif HAVE_TYPE_IN6_IFREQ
#elif HAVE_TYPE_LINUX_IN6_IFREQ
	ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	//
	// this could be done by the parent, which has
	// its own FD open anyway?
	//
	Socket s;
	Assert(s.create(PF_INET6, SOCK_DGRAM, 0));
	strncpy(ifr.ifr_name, name.cstr(), IFNAMSIZ);
	Must(ioctl(s.fd(), SIOCGIFINDEX, &ifr) >= 0);
	s.close();
	ifr6_ifindex = ifr.ifr_ifindex;
	memcpy(&ifr6_addr, addr.rawOctets(), sizeof(ifr6_addr));
	ifr6_prefixlen = mask.prefixlen();
#endif
}
