
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLPOPDISTRSYM_H
#define POLYGRAPH__PGL_PGLPOPDISTRSYM_H

#include "pgl/PglExprSym.h"

class String;
class PopDistr;

// XXX: should remove Pgl from file name

// popularity distribution
class PopDistrSym: public ExpressionSym {
	public:
		static const String TheType;

	public:
		PopDistrSym(const String &aType, PopDistr *aVal);
		virtual ~PopDistrSym();

		virtual bool isA(const String &type) const;

		PopDistr *val() const { return theVal; }

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

	protected:
		PopDistr *theVal;
};

#endif
