
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_INADDRESS_H
#define POLYGRAPH__XSTD_INADDRESS_H

#include "xstd/h/sys/types.h"
#include "xstd/h/netinet.h"
#include "xstd/h/sys/socket.h"


class InAddress {
	public:
		static InAddress IPvFour();
		static InAddress IPvSix();
		static InAddress NetMask(const int family, int bitCount);
		static unsigned short GetPort(const sockaddr_storage &s);
		static void SetPort(sockaddr_storage &s, int port);

	public:
		// InAddress is used in Arrays; make sure memset(0) works fine!
		InAddress();
		InAddress(const struct in_addr &a);
		InAddress(const struct in6_addr &a);
		InAddress(const struct sockaddr &sa);
		InAddress(const struct sockaddr_in &sa);
		InAddress(const struct sockaddr_in6 &sa);
		InAddress(const struct sockaddr_storage &sa);

		void init(const sockaddr_storage &s);

		bool known() const { return theFamily > 0 && isKnown; }

		// family() only used in calls to Socket::create()
		int family() const { return theFamily; }

		sockaddr_in sockAddrFour(int port) const;
		sockaddr_in6 sockAddrSix(int port) const;
		struct sockaddr_storage sockAddr(int port) const;

		int octet(int idx) const;
		int roctet(int idx) const;

		bool sameSubnet(const InAddress &other, int mask_bits) const;
		bool operator == (const InAddress &other) const;
		int compare(const InAddress &other) const;

		InAddress broadcast(const InAddress &aNetmask) const;
		unsigned int hash0() const;
		unsigned int hash1() const;

		int prefixlen() const;
		int len() const;  // in bytes

		void known(bool be) { isKnown = be; }

		int pton(const char *p);
		const char *rawImage() const; // returns pointer to shared buffer
		const char *image() const;    // '[' rawImage() ']' (for IPv6)
		const unsigned char *rawOctets() const;

	private:
		unsigned char *rawOctets();

	private:
		struct in6_addr theIPvSix;
		struct in_addr theIPvFour;
		int theFamily;
		bool isKnown;
};

#endif /* XSTD_IN_ADDRESS_H */
