
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"

#include "pgl/PglBoolSym.h"
#include "pgl/PglNumSym.h"



const String NumSym::TheType = "float";



NumSym::NumSym(double aVal): ExpressionSym(TheType), theVal(aVal) {
}

bool NumSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}


SynSym *NumSym::dupe(const String &type) const {
	if (isA(type))
		return new NumSym(theVal);

	return ExpressionSym::dupe(type);
}

ExpressionSym *NumSym::unOper(const Oper &op) const {
	if (op.plus())
		return new NumSym(+theVal);
	if (op.minus())
		return new NumSym(-theVal);
	return ExpressionSym::unOper(op);
}


ExpressionSym *NumSym::bnOper(const Oper &op, const SynSym &s) const {
	double otherVal = 0;
	
	if (s.isA(TheType)) {
		otherVal = ((const NumSym&)s.cast(TheType)).theVal;
	} else {
		NumSym *fs = (NumSym*)s.clone(TheType);
		if (!fs)
			return ExpressionSym::bnOper(op, s);
		otherVal = fs->theVal;
		delete fs;
	}

	if (op.div()) {
		checkDenom(otherVal);
		return new NumSym(theVal / otherVal);
	}

	if (op.lessTrue())
		return new BoolSym(theVal < otherVal);
	if (op.lessOrEq())
		return new BoolSym(theVal <= otherVal);
	if (op.greaterTrue())
		return new BoolSym(theVal > otherVal);
	if (op.greaterOrEq())
		return new BoolSym(theVal >= otherVal);
	if (op.plus())
		return new NumSym(theVal + otherVal);
	if (op.minus())
		return new NumSym(theVal - otherVal);
	if (op.mult())
		return new NumSym(theVal * otherVal);
	if (op.power())
		return new NumSym(pow(theVal, otherVal));

	return ExpressionSym::bnOper(op, s);
}

ostream &NumSym::print(ostream &os, const String &) const {
	return os << theVal;
}
