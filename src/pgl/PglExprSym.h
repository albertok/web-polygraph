
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLEXPRSYM_H
#define POLYGRAPH__PGL_PGLEXPRSYM_H

#include "xparser/SynSym.h"
#include "pgl/PglExprOper.h"

// base class for all kind of "expressions"
class ExpressionSym: public SynSym {
	public:
		typedef ExpressionOper Oper;

	public:
		static String TheType;

	public:
		ExpressionSym(const String &aType): SynSym(aType) {}

		// clone self and apply a given operator (unary or binary);
		// by default, complains about unsupported operation
		virtual ExpressionSym *unOper(const Oper &op) const;
		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		virtual bool isA(const String &type) const;

	protected:
		void checkDenom(int val) const;
		void checkDenom(double val) const;
};

#endif
