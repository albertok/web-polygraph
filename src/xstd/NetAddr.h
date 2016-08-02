
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_NETADDR_H
#define POLYGRAPH__XSTD_NETADDR_H

#include "xstd/InAddress.h"
#include "xstd/String.h"

// inet_addr wrapper which also stores port and
// does _lazy_ ntoa and aton manipulations
class NetAddr {
	public:
		NetAddr(): theAddrN(InAddress::IPvFour()), thePort(-1) {}
		NetAddr(const NetAddr &na): theAddrA(na.theAddrA), theAddrN(na.theAddrN), thePort(na.thePort) {}
		NetAddr(const String &addr, int aPort): theAddrA(addr), theAddrN(InAddress::IPvFour()), thePort(aPort) {}
		NetAddr(const InAddress &addr, int aPort): theAddrN(addr), thePort(aPort) {}
		NetAddr(const struct sockaddr_storage &ss);

		struct sockaddr_storage sockAddr() const;

		bool knownAddr() const { return theAddrA.len() || theAddrN.known(); }
		bool known() const { return knownAddr() || thePort >= 0; }
		operator void*() const { return known() ? (void*)-1 : 0; }
		bool operator == (const NetAddr &addr) const;
		bool operator != (const NetAddr &addr) const { return !(*this == addr); }
		bool operator < (const NetAddr &addr) const { return compare(addr) < 0; }
		bool sameButPort(const NetAddr &addr) const;
		int compare(const NetAddr &addr) const; // suitable for qsorting

		const String &addrA() const { if (!theAddrA) syncA(); return theAddrA; }
		const InAddress &addrN() const { if (!theAddrN.known()) syncN(); return theAddrN; }

		unsigned int hash0() const;
		unsigned int hash1() const;
		int octet(int idx) const; // a.b.c.d = 0.1.2.3
		int roctet(int idx) const; // octet from reverse/right side
		int port() const { return thePort; }

		void addr(const String &addr) { theAddrA = addr; theAddrN.known(false); }
		void addr(const InAddress &addr) { theAddrN = addr; theAddrA = 0; }
		void port(int aPort) { thePort = aPort; }
		void resolve(const InAddress &addr) { theAddrN = addr; }

		ostream &print(ostream &os) const;

		bool isDomainName() const;
		bool isDynamicName() const;
		bool suffixMatches(const NetAddr &addr) const;

		// for [persistent] storage only
		String &rawAddrA() const { return theAddrA; }
		InAddress &rawAddrN() const { return theAddrN; }

	protected:
		void syncA() const; // theAddrA <- theAddrN
		void syncN() const; // if (!isDomainName) theAddrN <- theAddrA

	protected:
		mutable String theAddrA;    // FQDN or IP; set if not null
		mutable InAddress theAddrN; // IP if available; set if .known()
		int thePort;                // set if >= 0;
};

inline ostream &operator <<(ostream &os, const NetAddr &addr) { return addr.print(os); }

#endif
