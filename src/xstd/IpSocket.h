
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_IPSOCKET_H
#define POLYGRAPH__XSTD_IPSOCKET_H

#include "xstd/Array.h"
#include "xstd/Socket.h"

struct ip_fw;
struct dn_pipe;

// this Socket type adds some basic IP-level operations to generic Socket
class IpSocket: public Socket {
	public:
		IpSocket() {}
		IpSocket(int aFD): Socket(aFD) {}

		bool create();

		/* options specific to IP */
		bool includeIpHdr(bool set);

		/* ipfw */
		bool getIpFwRules(Array<ip_fw> &rules) const;
		bool addIpFwRule(const ip_fw &rule);
		bool delIpFwRule(const ip_fw &rule);
		bool delIpFwRules();

		/* dummynet */
		bool getDnPipes(Array<dn_pipe> &pipes) const;
		bool configureDnPipe(const dn_pipe &pipe);
		bool delDnPipe(const dn_pipe &pipe);
		bool delDnPipes();
};

#endif
