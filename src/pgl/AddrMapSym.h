
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_ADDRMAPSYM_H
#define POLYGRAPH__PGL_ADDRMAPSYM_H

#include "xstd/Array.h"
#include "pgl/PglRecSym.h"

class NetAddr;

// parameters for object life cycle model
class AddrMapSym: public RecSym {
	public:
		static String TheType;

	public:
		AddrMapSym();
		AddrMapSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool usable() const; // can be use()d

		String zone() const;
		bool addresses(Array<NetAddr*> &addrs) const;
		bool names(Array<NetAddr*> &nms) const;
		const ArraySym *addressesSym() const;
		const ArraySym *namesSym() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
