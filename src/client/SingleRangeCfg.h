
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_SINGLERANGECFG_H
#define POLYGRAPH__CLIENT_SINGLERANGECFG_H

#include "pgl/pgl.h"
#include "xstd/BigSize.h"

#include "client/RangeCfg.h"

class SingleRangeSym;

// Manages single range spec configuration such as "a-b", "a-" or "-a"
class SingleRangeCfg: public RangeCfg {
	public:
		SingleRangeCfg();

		void configure(const SingleRangeSym &aSingleRange);
		virtual RangesInfo makeRangeSet(HttpPrinter &hp, const ObjId &oid, ContentCfg &contentCfg) const;

	protected:
		BigSize theFirstByteAbsolute;
		double theFirstByteRelative;
		BigSize theLastByteAbsolute;
		double theLastByteRelative;
		BigSize theSuffixLengthAbsolute;
		double theSuffixLengthRelative;
};

#endif
