
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_VERTHREEASSYM_H
#define POLYGRAPH__PGL_VERTHREEASSYM_H

#include "pgl/AddrSchemeSym.h"

class BenchSideSym;

// common class for all address schemes in version -3 workloads
class VerThreeAsSym: public AddrSchemeSym {
	public:
		static const String TheType;

	public:
		VerThreeAsSym(const String &aType, PglRec *aRec);

	protected:
		String addresses(const BenchSideSym *side, int shift, ArraySym *&addrs) const;
		ArraySym *ipStrToArr(const String &str) const;
};

#endif
