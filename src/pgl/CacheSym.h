
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_CACHESYM_H
#define POLYGRAPH__PGL_CACHESYM_H

#include "pgl/PglRecSym.h"

class BigSize;

// base class for robot and server symbols
class CacheSym: public RecSym {
	public:
		static String TheType;

	public:
		CacheSym();
		CacheSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool icpPort(int &port) const;
		bool capacity(BigSize &cap) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
