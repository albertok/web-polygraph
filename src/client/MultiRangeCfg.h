
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_MULTIRANGECFG_H
#define POLYGRAPH__CLIENT_MULTIRANGECFG_H

#include "pgl/pgl.h"

#include "client/RangeCfg.h"

class MultiRangeSym;
class RndDistr;

// Manages multi range spec configuration such as "a-b,c-d"
class MultiRangeCfg: public RangeCfg {
	public:
		MultiRangeCfg();

		void configure(const MultiRangeSym &aMultiRange);
		virtual RangesInfo makeRangeSet(HttpPrinter &hp, const ObjId &oid, ContentCfg &contentCfg) const;

	protected:
		RndDistr *theGapLengthAbsolute;
		RndDistr *theGapLengthRelative;
		RndDistr *theFirstRangeStartAbsolute;
		RndDistr *theFirstRangeStartRelative;
		RndDistr *theRangeLengthAbsolute;
		RndDistr *theRangeLengthRelative;
		RndDistr *theRangeCount;

	private:
		int calculateGap(const int repSize) const;
};

#endif
