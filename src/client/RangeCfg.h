
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_RANGECFG_H
#define POLYGRAPH__CLIENT_RANGECFG_H

#include "xstd/Size.h"

class ContentCfg;
class HttpPrinter;
class ObjId;

// Abstract base class for SingleRangeCfg and MultiRangeCfg
class RangeCfg {
	public:
		virtual ~RangeCfg();

		struct RangesInfo {
			Size theTotalSize;
			unsigned int theCount;
		};

		virtual RangesInfo makeRangeSet(HttpPrinter &hp, const ObjId &oid, ContentCfg &contentCfg) const = 0;
};

#endif
