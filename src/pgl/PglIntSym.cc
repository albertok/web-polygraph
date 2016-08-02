
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"
#include <limits.h>

#include "pgl/PglBoolSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglRateSym.h"
#include "pgl/PglBwidthSym.h"
#include "pgl/PglDistrSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglIntSym.h"

#include "xstd/gadgets.h"


String IntSym::TheType = "int";


IntSym::IntSym(const Value aVal = -1): ExpressionSym(TheType), theVal(aVal) {
}

bool IntSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *IntSym::dupe(const String &type) const {
	if (isA(type))
		return new IntSym(theVal);

	if (type == "float")
		return new NumSym(theVal);

	if (type == BoolSym::TheType)
		return new BoolSym(theVal != 0);

	// zero is zero regardless of strScale
	if (!theVal) {
		if (type == TimeSym::TheType)
			return new TimeSym(Time(0,0));
		if (type == SizeSym::TheType)
			return new SizeSym(BigSize(0));
		if (type == RateSym::TheType)
			return new RateSym(0);
		if (type == BwidthSym::TheType)
			return new BwidthSym(0);
		if (type.str("_distr"))
			return new DistrSym(type, 0);
	}

	return ExpressionSym::dupe(type);
}

ExpressionSym *IntSym::unOper(const Oper &op) const {
	if (op.plus())
		return new IntSym(+theVal);
	if (op.minus())
		return new IntSym(-theVal);
	if (op.boolNot())
		return new BoolSym(!theVal);
	return ExpressionSym::unOper(op);
}


ExpressionSym *IntSym::bnOper(const Oper &op, const SynSym &s) const {
	double otherVal = 0;

	if (s.isA(TheType)) {
		otherVal = ((const IntSym&)s.cast(TheType)).val();
	} else 
	if (IntSym *is = (IntSym*)s.clone(TheType)) {
		otherVal = is->val();
		delete is;
	} else
		return ExpressionSym::bnOper(op, s);

	if (op.div()) {
		checkDenom(otherVal);
		return fit(op, theVal / otherVal);
	}
	if (op.mod()) {
		checkDenom((int)otherVal);
		return fit(op, theVal % (int)otherVal);
	}
	if (op.range())
		return operRange((int)otherVal);

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
		return fit(op, theVal + otherVal);
	if (op.minus())
		return fit(op, theVal - otherVal);
	if (op.mult())
		return fit(op, theVal * otherVal);
	if (op.power())
		return fit(op, pow(theVal, otherVal));
	return ExpressionSym::bnOper(op, s);
}

IntSym *IntSym::fit(const Oper &op, double d) const {
	return Fit(op, d, theLoc);
}

ExpressionSym *IntSym::operRange(int otherEnd) const {
	ArraySym *arr = new ArraySym(TheType);

	const int step = theVal <= otherEnd ? +1 : -1;
	// not optimized; could use IntRangeArrayItem instead
	for (Value i = theVal; step*i <= step*otherEnd; i += step) {
		arr->add(IntSym(i)); 
	}

	Assert(arr->count()); // empty ranges are probably impossible
	return arr;
}

ostream &IntSym::print(ostream &os, const String &) const {
	return os << theVal;
}


IntSym *IntSym::Fit(const Oper &op, double d, const TokenLoc &loc) {
	if (fabs(d) > INT_MAX)
		cerr << loc << "integer overflow as a result of `" 
			<< op.image() << "' (|" << d << "| > " << INT_MAX << ")" 
			<< endl << xexit;
	return new IntSym((int)d);
}
