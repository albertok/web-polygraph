
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_POLYMIX3ASSYM_H
#define POLYGRAPH__PGL_POLYMIX3ASSYM_H

#include "pgl/VerThreeAsSym.h"

// PolyMix-3 addressing scheme
class PolyMix3AsSym: public VerThreeAsSym {
	public:
		static const String TheType;

	public:
		PolyMix3AsSym();
		PolyMix3AsSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		virtual String robots(ArraySym *&addrs) const;
		virtual String servers(ArraySym *&addrs) const;
};

#endif
