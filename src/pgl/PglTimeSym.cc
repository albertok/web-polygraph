
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"

#include "pgl/PglBoolSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglTimeSym.h"



String TimeSym::TheType = "time";



TimeSym::TimeSym(Time aVal): ExpressionSym(TheType), theVal(aVal) {
}

bool TimeSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}


SynSym *TimeSym::dupe(const String &type) const {
	if (isA(type))
		return new TimeSym(theVal);
	return ExpressionSym::dupe(type);
}

ExpressionSym *TimeSym::unOper(const Oper &op) const {
	if (op.plus())
		return new TimeSym(+theVal);
	if (op.minus())
		return new TimeSym(-theVal);
	return ExpressionSym::unOper(op);
}

ExpressionSym *TimeSym::bnOper(const Oper &op, const SynSym &s) const {
	// division is defined for both Time and Num
	const bool forceTT = op.div() && s.canBe(TheType);

	if (op.comparison() || op.plus() || op.minus() || forceTT)
		return operTT(op, s); // time op time

	if (op.mult() || op.div())
		return operTN(op, s); // time op num
	
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *TimeSym::operTT(const Oper &op, const SynSym &s) const {
	Time otherVal;
	
	if (s.isA(TheType)) {
		otherVal = ((const TimeSym&)s.cast(TheType)).val();
	} else {
		TimeSym *ts = (TimeSym*)s.clone(TheType);
		if (!ts)
			return ExpressionSym::bnOper(op, s);
		otherVal = ts->val();
		delete ts;
	}

	if (op.div()) {
		checkDenom(otherVal != 0);
		return new NumSym(theVal / otherVal);
	}

	if (op.same())
		return new BoolSym(theVal == otherVal);
	if (op.diff())
		return new BoolSym(theVal != otherVal);
	if (op.lessTrue())
		return new BoolSym(theVal < otherVal);
	if (op.lessOrEq())
		return new BoolSym(theVal <= otherVal);
	if (op.greaterTrue())
		return new BoolSym(theVal > otherVal);
	if (op.greaterOrEq())
		return new BoolSym(theVal >= otherVal);
	if (op.plus())
		return new TimeSym(theVal + otherVal);
	if (op.minus())
		return new TimeSym(theVal - otherVal);
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *TimeSym::operTN(const Oper &op, const SynSym &s) const {
	double otherVal = 0;
	
	if (s.isA(NumSym::TheType)) {
		otherVal = ((const NumSym&)s.cast(NumSym::TheType)).val();
	} else {
		NumSym *fs = (NumSym*)s.clone(NumSym::TheType);
		if (!fs)
			return ExpressionSym::bnOper(op, s);
		otherVal = fs->val();
		delete fs;
	}

	if (op.div()) {
		checkDenom(otherVal);
		return new TimeSym(theVal / otherVal);
	}

	if (op.mult())
		return new TimeSym(theVal * otherVal);

	return ExpressionSym::bnOper(op, s);
}

ostream &TimeSym::print(ostream &os, const String &) const {
	return os << theVal;
}
