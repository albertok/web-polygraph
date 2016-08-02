
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSTRARRAYBLOCKSYM_H
#define POLYGRAPH__PGL_PGLSTRARRAYBLOCKSYM_H

#include "pgl/PglContainerSym.h"

class StringArrayBlock;

// a base class for various compact representations of string array classes
// kids maintain a StringArrayBlock of some kind
// TODO: merge and move more of StrRangeSym and CredArrSym here?
class StringArrayBlockSym: public ContainerSym {
	public:
		static const String TheType;

	public:
		StringArrayBlockSym(const String &aType);

		virtual bool isA(const String &type) const;

		virtual void forEach(Visitor &v, RndGen *const rng = 0) const;
		virtual const StringArrayBlock &val() const = 0;
};

#endif
