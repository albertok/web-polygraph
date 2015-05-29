
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_DUMPERSYM_H
#define POLYGRAPH__PGL_DUMPERSYM_H

#include "pgl/PglRecSym.h"

// console configuration
class DumperSym: public RecSym {
	public:
		static String TheType;

	public:
		DumperSym();
		DumperSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
