
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLINTSYM_H
#define POLYGRAPH__PGL_PGLINTSYM_H

#include "pgl/PglExprSym.h"

class IntSym: public ExpressionSym {
	public:
		static String TheType;
		static IntSym *Fit(const Oper &op, double d, const TokenLoc &);

	public:
		IntSym(int aVal);

		int val() const { return theVal; }
		void val(int v) { theVal = v; }

		virtual bool isA(const String &type) const;

		virtual ExpressionSym *unOper(const Oper &op) const;
		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
		IntSym *fit(const Oper &op, double d) const;
		ExpressionSym *operRange(int otherEnd) const;

	protected:
		int theVal;
};

#endif
