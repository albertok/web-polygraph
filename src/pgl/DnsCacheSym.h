
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_DNS_CACHESYM_H
#define POLYGRAPH__PGL_DNS_CACHESYM_H

#include "pgl/PglRecSym.h"

// DNS caching configuration (used by Robots)
class DnsCacheSym: public RecSym {
	public:
		static String TheType;

	public:
		DnsCacheSym();
		DnsCacheSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool capacity(int &cap) const;
		String sharingGroup() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
