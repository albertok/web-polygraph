
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLREGEXSYM_H
#define POLYGRAPH__PGL_PGLREGEXSYM_H

#include "pgl/PglExprSym.h"

class String;
class RegEx;

// with most expressions we want to convert the result back to the
// original domain: "5+5" becomes "10". We could do that with RE, but
// future matching optimizations and presentation issues make that a bad idea
// instead, we want to use "RE expression" so that operations on 
// "RE expression"s remain in the "RE expression" domain
class RegExExpr {
	public:
		enum Oper { opNone = 0, opNot, opAnd, opOr };

	public:
		RegExExpr(RegEx *aVal);
		RegExExpr(RegExExpr *aLhs, Oper anOper, RegExExpr *aRhs);

		ostream &print(ostream &os) const;

		RegEx *theVal;       // if theOper == opNone
		RegExExpr *theLhs;   // if theOper != opNone
		RegExExpr *theRhs;   // if theOper != opNone && theOper != opNot
		int theOper;
};

// a RE wrapper
class RegExSym: public ExpressionSym {
	public:
		static String TheType;

	public:
		RegExSym(RegExExpr *aVal);
		virtual ~RegExSym();

		virtual bool isA(const String &type) const;

		virtual ExpressionSym *unOper(const Oper &op) const;
		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		RegExExpr *val() const { return theVal; }

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

	protected:
		mutable RegExExpr *theVal;
};

#endif
