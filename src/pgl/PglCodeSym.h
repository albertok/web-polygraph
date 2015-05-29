
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLCODESYM_H
#define POLYGRAPH__PGL_PGLCODESYM_H

#include "xparser/ParsSym.h"

// a piece of PGL code for later execution/interpretation
class CodeSym: public ParsSym {
	public:
		static String TheType;

	public:
		CodeSym(const ParsSym &rawCode);

		virtual bool isA(const String &type) const;

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &) const;
};

#endif
