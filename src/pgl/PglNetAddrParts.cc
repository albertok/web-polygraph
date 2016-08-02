
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include <ctype.h>

#include "xstd/Assert.h"
#include "xstd/gadgets.h"
#include "base/AnyToString.h"
#include "pgl/PglNetAddrParts.h"


PglNetAddrParts::PglNetAddrParts(const String &anImage): theImage(anImage),
	theHostKind(hkNone), theSubnet(-1) {
	theBuf = theImage;
	(void)parse();
}

// splits image into known parts and sets the address kind appropriately
bool PglNetAddrParts::parse() {
	// the order is important
	return parseIfname() && determineHostKind() &&
		parseSubnet() && parseHosts() && parsePorts();
}

// also determined address kind
bool PglNetAddrParts::parseIfname() {
	// We have a conflict with '::'.  It could be either
	// an ifname, or part of an IPv6 address.  We require
	// IPv6 addrs be delimited by [].  So, if there is no
	// square bracket before the occurance of '::', we
	// have an ifname, not an IPv6 address.

	const char *columns = theBuf.str("::"); // ifname end?
	const char *bracket = theBuf.chr('['); // IPv6 start?

	if (columns && (!bracket || columns < bracket)) { // ifname
		theIfname = theBuf(0, columns - theBuf.cstr());
		theBuf = theBuf(columns+2 - theBuf.cstr(), theBuf.len());

		if (!theIfname.len()) {
			theError = "empty interface name at the start of " + theBuf;
			return false;
		}
	}
	return true;
}

bool PglNetAddrParts::determineHostKind() {
	// IPv6 must start with a '[' followed by a hex number or ':'
	if (theBuf.len() > 1 && theBuf[0] == '[' &&
		(isxdigit(theBuf[1]) || theBuf[1] == ':')) {
		theHostKind = hkIPv6;
		return true;
	}

	// '[' is not allowed in IPv4 addresses, no need to skip
	if (theBuf.len() > 0 && isdigit(theBuf[0])) {
		theHostKind = hkIPv4;
		return true;
	}

	// may start with '[' but we do not care
	if (theBuf.len() > 0) {
		theHostKind = hkDomainName;
		return true;
	}

	theError = "empty domain name or IP address?";
	return false;
}

bool PglNetAddrParts::parseSubnet() {
	if (const char *slash = theBuf.chr('/')) {
		if (theHostKind == hkDomainName) {
			theError = String("domain names cannot have subnets, "
				"but found one after ") + slash;
			return false;
		}

		const char *send = 0;
		if (!isInt(slash+1, theSubnet, &send, 10)) {
			theError = String("cannot parse subnet after ") + slash;
			return false;
		}

		if (*send) {
			theError = String("garbage following subnet near ") + send;
			return false;
		}

		Should(theHostKind == hkIPv4 || theHostKind == hkIPv6);
		const int maxSubnet = theHostKind == hkIPv4 ? 32 : 128;
		if (!(1 <= theSubnet && theSubnet <= maxSubnet)) {
			theError = String("subnet ") + AnyToString(theSubnet) +
				" is larger than maximum subnet for this IP type (" +
				AnyToString(maxSubnet) + ")";
			return false;
		}

		theBuf = theBuf(0, slash - theBuf.cstr());
	}

	return true;
}

bool PglNetAddrParts::parseHosts() {
	if (theHostKind == hkIPv6) {
		if (const char *hostsEnd = theBuf.chr(']')) {
			hostsEnd++;
			theHosts = theBuf(0, hostsEnd - theBuf.cstr());
			theBuf = theBuf(hostsEnd - theBuf.cstr(), theBuf.len());
			return true;
		} else {
			theError = "IPv6 address is not terminated by ']' near " + theBuf;
			return false;
		}
	} else {
		const char *ports = theBuf.chr(':');
		if (!ports)
			ports = theBuf.cstr() + theBuf.len(); // end of string
		theHosts = theBuf(0, ports - theBuf.cstr());
		if (!theHosts.len()) {
			theError = "empty network address near " + theBuf;
			return false;
		}
		theBuf = theBuf(ports - theBuf.cstr(), theBuf.len());
		return true;
	}
}

bool PglNetAddrParts::parsePorts() {
	if (!theBuf.len() || (theBuf[0] == ':' && theBuf.len() > 1)) {
		thePorts = theBuf;
		theBuf = String();
		return true;
	} else {
		theError = "garbage at the end of address near " + theBuf;
		return false;
	}	
}

bool PglNetAddrParts::single() const {
	if (thePorts.chr('-'))
		return false;

	if (theHostKind == hkIPv6 || theHostKind == hkIPv4)
		return !theHosts.chr('-');

	if (theHostKind == hkDomainName)
		return !theHosts.chr('[');

	return Should(false);
}

const String &PglNetAddrParts::host() const {
	Should(single());
	return hosts();
}

int PglNetAddrParts::port() const {
	if (!Should(single()))
		return -1;

	int port = -1;
	if (thePorts.len() > 1 && isInt(thePorts.cstr()+1, port))
		return port;

	return -1;
}
