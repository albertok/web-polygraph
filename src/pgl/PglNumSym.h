
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLNUMSYM_H
#define POLYGRAPH__PGL_PGLNUMSYM_H

#include "pgl/PglExprSym.h"

// floating point "number"

class NumSym: public ExpressionSym {
	public:
		static const String TheType;

	public:
		NumSym(double aVal);

		double val() const { return theVal; }
		void val(double v) { theVal = v; }

		virtual bool isA(const String &type) const;

		virtual ExpressionSym *unOper(const Oper &op) const;
		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

	protected:
		double theVal;
};

#endif
