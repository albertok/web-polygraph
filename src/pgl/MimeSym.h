
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_MIMESYM_H
#define POLYGRAPH__PGL_MIMESYM_H

#include "xstd/Array.h"
#include "pgl/PglRecSym.h"

// parameters for object life cycle model
class MimeSym: public RecSym {
	public:
		static String TheType;

	public:
		MimeSym();
		MimeSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		String mimeType() const;
		bool extensions(Array<String*> &exts, RndDistr *&sel) const;
		bool prefixes(Array<String*> &pxs, RndDistr *&sel) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
