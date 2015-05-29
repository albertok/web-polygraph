
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_MULTIRANGESYM_H
#define POLYGRAPH__PGL_MULTIRANGESYM_H

#include "xstd/String.h"
#include "pgl/RangeSym.h"

class MultiRangeSym: public RangeSym {
	public:
		typedef bool (MultiRangeSym::*FactorFunc)(double &rf) const;

	public:
		static const String TheType;

	public:
		MultiRangeSym();
		MultiRangeSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		RndDistr *first_range_start_absolute() const;
		RndDistr *first_range_start_relative() const;
		RndDistr *range_length_absolute() const;
		RndDistr *range_length_relative() const;
                RndDistr *range_count() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
