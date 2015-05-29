
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLNETADDRPARTS_H
#define POLYGRAPH__PGL_PGLNETADDRPARTS_H

#include "xstd/String.h"

class PglNetAddrParts {
	public:
		enum HostKind { hkNone, hkDomainName, hkIPv4, hkIPv6 };

	public:
		PglNetAddrParts(const String &anImage);

		const String &error() const { return theError; }
		bool single() const;

		HostKind hostKind() const { return theHostKind; }
		const String &ifName() const { return theIfname; }
		const String &hosts() const { return theHosts; }
		const String &ports() const { return thePorts; }
		int subnet() const { return theSubnet; }

		// check that single() is true before calling these
		const String &host() const;
		int port() const;

	protected:
		bool parse();
		bool parseIfname();
		bool determineHostKind();
		bool parseSubnet();
		bool parseHosts();
		bool parsePorts();

	protected:
		String theImage;

		HostKind theHostKind;

		String theIfname;
		String theHosts;
		String thePorts;
		int theSubnet;

		String theError;
		String theBuf; // unparsed content
};

#endif
