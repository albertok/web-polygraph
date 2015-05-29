
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/Assert.h"
#include "xstd/String.h"
#include "xstd/RegEx.h"
#include "pgl/PglRegExSym.h"


String RegExSym::TheType = "re";


// XXX: we are leaking actual REs and REGroups as they are never deleted!


RegExExpr::RegExExpr(RegEx *aVal): theVal(aVal),
	theLhs(0), theRhs(0), theOper(opNone) {
}

RegExExpr::RegExExpr(RegExExpr *aLhs, Oper anOper, RegExExpr *aRhs):
	theLhs(aLhs), theRhs(aRhs), theOper(anOper) {
}

ostream &RegExExpr::print(ostream &os) const {
	if (theOper == opNone) {
		Assert(theVal);
		theVal->print(os);
	} else
	if (theOper == opNot) {
		Assert(theLhs);
		theLhs->print(os << "!(") << ')';
	} else
	if (theOper == opAnd) {
		Assert(theLhs && theRhs);
		theLhs->print(os << '(') << " && ";
		theRhs->print(os) << ')';
	} else
	if (theOper == opOr) {
		Assert(theLhs && theRhs);
		theLhs->print(os << '(') << " || ";
		theRhs->print(os) << ')';
	} else {
		Assert(false);
	}
	return os;
}


RegExSym::RegExSym(RegExExpr *aVal): ExpressionSym(TheType), theVal(aVal) {
}

RegExSym::~RegExSym() {
}

bool RegExSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *RegExSym::dupe(const String &type) const {
	if (isA(type))
		return new RegExSym(theVal);
	return ExpressionSym::dupe(type);
}

ExpressionSym *RegExSym::unOper(const Oper &op) const {
	if (op.boolNot())
		return new RegExSym(new RegExExpr(theVal, RegExExpr::opNot, 0));
	else
		return ExpressionSym::unOper(op);
}

ExpressionSym *RegExSym::bnOper(const Oper &op, const SynSym &s) const {
	if (!op.boolAnd() && !op.boolOr())
		return ExpressionSym::bnOper(op, s);

	RegExExpr *otherVal = 0;

	if (s.isA(TheType)) {
		otherVal = ((const RegExSym&)s.cast(TheType)).val();
	} else 
	if (RegExSym *clone = (RegExSym*)s.clone(TheType)) {
		otherVal = clone->val();
		delete clone;
	} else {
		return ExpressionSym::bnOper(op, s);
	}

	if (op.boolAnd())
		return new RegExSym(new RegExExpr(theVal, RegExExpr::opAnd, otherVal));
	if (op.boolOr())
		return new RegExSym(new RegExExpr(theVal, RegExExpr::opOr, otherVal));
	Assert(false);
	return 0;
}

ostream &RegExSym::print(ostream &os, const String &) const {
	if (theVal)
		theVal->print(os);
	else
		os << "<none>";
	return os;
}
