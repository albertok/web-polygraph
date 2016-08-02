
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_HOSTSBASEDSYM_H
#define POLYGRAPH__PGL_HOSTSBASEDSYM_H

#include "xstd/Array.h"
#include "pgl/PglRecSym.h"
#include "xstd/NetAddr.h"

class NetAddrSym;
class ArraySym;
class SslWrapSym;

// base class for robot and server symbols
class HostsBasedSym: public RecSym {
	public:
		static String TheType;

	public:
		HostsBasedSym(const String &aType);
		HostsBasedSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		int hostCount() const;
		NetAddrSym *host(int idx) const;

		const ArraySym *addresses() const; // may return null
		bool addresses(ArraySym &addrs) const;
		bool sslWraps(Array<SslWrapSym*> &syms, RndDistr *&sel) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
