
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLNETADDRRANGESYM_H
#define POLYGRAPH__PGL_PGLNETADDRRANGESYM_H

#include "pgl/PglContainerSym.h"

class PglNetAddrRange;

// a compact representation of a range of addresses
class NetAddrRangeSym: public ContainerSym {
	public:
		static const String TheType;

	public:
		NetAddrRangeSym();
		virtual ~NetAddrRangeSym();

		virtual bool isA(const String &type) const;

		virtual int count() const;
		virtual bool probsSet() const;
		virtual const SynSym *itemProb(int offset, double &prob) const;

		void range(PglNetAddrRange *aRange) { theRange = aRange; }

		virtual void forEach(Visitor &v, RndGen *const rng = 0) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &type) const;

	protected:
		PglNetAddrRange *theRange;
		mutable NetAddrSym *theAddr; // cache for itemProb()
};

#endif
