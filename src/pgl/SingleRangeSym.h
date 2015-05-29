
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_SINGLERANGESYM_H
#define POLYGRAPH__PGL_SINGLERANGESYM_H

#include "xstd/String.h"
#include "pgl/RangeSym.h"

class SingleRangeSym: public RangeSym {
	public:
		typedef bool (SingleRangeSym::*FactorFunc)(double &rf) const;

	public:
		static const String TheType;

	public:
		SingleRangeSym();
		SingleRangeSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool firstByteAbsolute(BigSize &sz) const;
		bool firstByteRelative(double &f) const;
		bool lastByteAbsolute(BigSize &sz) const;
		bool lastByteRelative(double &f) const;
		bool suffixLengthAbsolute(BigSize &sz) const;
		bool suffixLengthRelative(double &f) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
