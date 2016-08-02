
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLCLONERSYM_H
#define POLYGRAPH__PGL_PGLCLONERSYM_H

#include "pgl/PglContainerSym.h"

// a compact representation of N cloned items
class ClonerSym: public ContainerSym {
	public:
		static String TheType;

	public:
		ClonerSym(const SynSym &aSource, int aCount);
		virtual ~ClonerSym();

		virtual bool isA(const String &type) const;

		virtual int count() const;
		virtual bool probsSet() const;
		virtual const SynSym *itemProb(int offset, double &prob) const;

		virtual void forEach(Visitor &v, RndGen *const rng = 0) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

		const SynSym *cloneSource() const { return theSource; }
		int cloneFactor() const { return theCount; }

	protected:
		virtual SynSym *dupe(const String &type) const;

	protected:
		SynSym *theSource;
		int theCount;
};

#endif
