
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_OBJLIFECYCLESYM_H
#define POLYGRAPH__PGL_OBJLIFECYCLESYM_H

#include "xstd/Array.h"
#include "pgl/PglRecSym.h"

class QualifSym;

// parameters for object life cycle model
class ObjLifeCycleSym: public RecSym {
	public:
		static String TheType;

	public:
		ObjLifeCycleSym();
		ObjLifeCycleSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		RndDistr *bday() const;
		RndDistr *length() const;
		bool variance(double &ratio) const;
		bool withLmt(double &ratio) const;
		bool expires(Array<RndDistr*> &times, RndDistr *&selector, Array<QualifSym*> &qs) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
