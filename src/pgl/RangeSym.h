
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_RANGESYM_H
#define POLYGRAPH__PGL_RANGESYM_H

#include "xstd/String.h"
#include "pgl/PglRecSym.h"

class RangeSym: public RecSym {
	public:
		typedef bool (RangeSym::*FactorFunc)(double &rf) const;

	public:
		static const String TheType;

	public:
		RangeSym();
		RangeSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
