
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

// do not compile without ip_fw.h
#if HAVE_NETINET_IP_FW_H && USE_DUMMYNET

#include "xstd/h/net/if.h"
#include "xstd/h/netinet.h"
#include "xstd/h/sys/socket.h"

#include <netinet/ip_fw.h>

#include <net/if.h>
#include <net/route.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <netinet/ip_dummynet.h>

#include "xstd/IpSocket.h"


// this will be a member when template members are supported
// see if this should be moved to basic Socket
template <class Item>
static
bool IpSocketGetArrayOpt(const IpSocket &s, int optName, Array<Item> &arr) {
	// we have to guess the number of items
	arr.stretch(4*1024); // have to start somewhere
	do {
		arr.stretch(arr.capacity()*2);

		socklen_t size = arr.size();
		if (!s.getOpt(IPPROTO_IP, optName, arr.items(), &size))
			return false;
		arr.count(size / sizeof(dn_pipe));

	} while (arr.count() == arr.capacity());
	return true;
}


bool IpSocket::create() {
	return Socket::create(PF_INET, SOCK_RAW, 0);
}

bool IpSocket::includeIpHdr(bool set) {
#ifdef IP_HDRINCL
	const int opt = (int)set;
	return setOpt(IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt));
#else
	return unsupOpt(&set);
#endif
}


bool IpSocket::getIpFwRules(Array<ip_fw> &rules) const {
#ifdef IP_FW_GET
	return IpSocketGetArrayOpt(*this, IP_FW_GET, rules);
#else
	return unsupOpt(rules.items());
#endif
}

bool IpSocket::delIpFwRule(const ip_fw &rule) {
#ifdef IP_FW_DEL
	return setOpt(IPPROTO_IP, IP_FW_DEL, &rule, sizeof(rule));
#else
	return unsupOpt(&rule ? 0 : 0);
#endif
}

bool IpSocket::delIpFwRules() {
#ifdef IP_FW_FLUSH
	return setOpt(IPPROTO_IP, IP_FW_FLUSH, 0, 0);
#else
	return unsupOpt();
#endif
}

bool IpSocket::addIpFwRule(const ip_fw &rule) {
#ifdef IP_FW_ADD
	return setOpt(IPPROTO_IP, IP_FW_ADD , &rule, sizeof(rule));
#else
	return unsupOpt(&rule ? 0 : 0);
#endif
}


/* dummynet wrappers */

bool IpSocket::getDnPipes(Array<dn_pipe> &pipes) const {
#ifdef IP_DUMMYNET_GET
	return IpSocketGetArrayOpt(*this, IP_DUMMYNET_GET, pipes);
#else
	return unsupOpt(pipes.items());
#endif
}

bool IpSocket::configureDnPipe(const dn_pipe &pipe) {
#ifdef IP_DUMMYNET_CONFIGURE
	return setOpt(IPPROTO_IP, IP_DUMMYNET_CONFIGURE, &pipe, sizeof(pipe));
#else
	return unsupOpt(&pipe ? 0 : 0);
#endif
}

bool IpSocket::delDnPipe(const dn_pipe &pipe) {
#ifdef IP_DUMMYNET_DEL
	return setOpt(IPPROTO_IP, IP_DUMMYNET_DEL, &pipe, sizeof(pipe));
#else
	return unsupOpt(&pipe ? 0 : 0);
#endif
}

bool IpSocket::delDnPipes() {
#ifdef IP_DUMMYNET_FLUSH
	return setOpt(IPPROTO_IP, IP_DUMMYNET_FLUSH, 0, 0);
#else
	return unsupOpt();
#endif
}


#endif
