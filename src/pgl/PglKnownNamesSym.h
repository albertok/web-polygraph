
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLKNOWNNAMESSYM_H
#define POLYGRAPH__PGL_PGLKNOWNNAMESSYM_H

#include "pgl/PglArraySym.h"

// array of symbol names (i.e. array of references)
class KnownNamesSym: public ArraySym {
	public:
		static String TheType;

	public:
		KnownNamesSym(const String &anItemType);

		virtual bool isA(const String &type) const;

	protected:
		virtual ArraySym *create(const String &itemType) const;
};

#endif
