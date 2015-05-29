
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLNETADDRSYM_H
#define POLYGRAPH__PGL_PGLNETADDRSYM_H

#include "xstd/String.h"
#include "xstd/NetAddr.h"
#include "pgl/PglRecSym.h"

class SynSymTblItem;
class StringSym;

// host:port network address

class NetAddrSym: public RecSym {
	public:
		static String TheType;

	public:
		NetAddrSym();
		NetAddrSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		NetAddr val() const;
		void val(const NetAddr &addr);
		void portVal(int newPort);

		String ifName() const;
		bool subnet(int &sn) const;

		void setIfname(const String &aName);
		void setSubnet(int aSubnet);

		void printUnquoted(ostream &os) const;
		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
