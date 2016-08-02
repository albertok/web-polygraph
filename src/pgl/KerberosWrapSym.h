
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_KERBEROSWRAPSYM_H
#define POLYGRAPH__PGL_KERBEROSWRAPSYM_H

#include "pgl/PglRecSym.h"


// a set of KDC addresses and other parameters for Kerberos
class KerberosWrapSym: public RecSym {
	public:
		static const String TheType;

	public:
		KerberosWrapSym();
		KerberosWrapSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		String realm() const;
		bool servers(Array<NetAddr*> &addrs) const;
		bool serversTcp(Array<NetAddr*> &addrs) const;
		bool serversUdp(Array<NetAddr*> &addrs) const;
		Time timeout() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
