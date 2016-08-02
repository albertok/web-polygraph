
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_NETIFACE_H
#define POLYGRAPH__XSTD_NETIFACE_H

#include "xstd/String.h"
#include "xstd/Array.h"
#include "xstd/Socket.h"
#include "xstd/NetAddr.h"

#include "xstd/h/netinet.h"

class InetIfReq;

// retreives a list of all interface addresses using SIOCGIFCONF or equiv
class NetIface {
	public:
		class Primaries {
			public:
				NetAddr vFour;
				NetAddr vSix;
		};

	public:
		NetIface();
		~NetIface();

		const String &name() const { return theName; }
		bool primaries(Primaries &ips) const;
		int nextAliasIndex() const; // max alias index + 1 or 0 if no aliases configured

		void name(const String &aName);

		bool delAlias(const InAddress &addr, const InAddress &netmask, const String &name);
		int delAliases(); // returns the number of aliases deleted or -1

		bool addAlias(const InAddress &addrs, int idx, const InAddress &netmask);
		bool addAliases(const Array<InAddress> &addrs, const InAddress &netmask);

	protected:
		void getAliases(Array<InetIfReq> &aliases, Array<InAddress> *netmasks = 0) const;
		static void EjectPrimary(const NetAddr &primary, Array<InetIfReq> &all, Array<InAddress> *netmasks);

	protected:
		String theName;
		mutable Socket theV4Sock;
		mutable Socket theV6Sock;
};

#endif
