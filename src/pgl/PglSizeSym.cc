
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"

#include "pgl/PglBoolSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglRateSym.h"
#include "pgl/PglBwidthSym.h"
#include "pgl/PglSizeSym.h"



String SizeSym::TheType = "size";



SizeSym::SizeSym(const BigSize &aVal): ExpressionSym(TheType), theVal(aVal) {
}

bool SizeSym::isA(const String &type) const {
	return ExpressionSym::isA(type) ||
		type == TheType || type == TheType;
}


SynSym *SizeSym::dupe(const String &type) const {
	if (isA(type))
		return new SizeSym(theVal);
	return ExpressionSym::dupe(type);
}

ExpressionSym *SizeSym::unOper(const Oper &op) const {
	if (op.plus())
		return new SizeSym(+theVal);
	if (op.minus())
		return new SizeSym(-theVal);
	return ExpressionSym::unOper(op);
}


ExpressionSym *SizeSym::bnOper(const Oper &op, const SynSym &s) const {
	if (SizeSym *ss = (SizeSym*)s.clone(TheType))
		return operSS(op, ss, s);

	if (TimeSym *ts = (TimeSym*)s.clone(TimeSym::TheType))
		return operST(op, ts, s);

	if (RateSym *rs = (RateSym*)s.clone(RateSym::TheType))
		return operSR(op, rs, s);

	if (BwidthSym *bs = (BwidthSym*)s.clone(BwidthSym::TheType))
		return operSB(op, bs, s);

	if (NumSym *ns = (NumSym*)s.clone(NumSym::TheType))
		return operSN(op, ns, s);
	
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *SizeSym::operSS(const Oper &op, SizeSym *ss, const SynSym &s) const {
	BigSize otherVal = ss->val();
	delete ss;
	
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
		return new SizeSym(theVal + otherVal);
	if (op.minus())
		return new SizeSym(theVal - otherVal);
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *SizeSym::operST(const Oper &op, TimeSym *ts, const SynSym &s) const {
	Time otherInterval = ts->val();
	delete ts;

	if (op.div())
		return new BwidthSym(Bandwidth(theVal, otherInterval));
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *SizeSym::operSR(const Oper &op, RateSym *rs, const SynSym &s) const {
	double rps = rs->val();
	delete rs;

	if (op.mult())
 		return new BwidthSym(Bandwidth(theVal*rps, Time::Sec(1)));
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *SizeSym::operSB(const Oper &op, BwidthSym *bs, const SynSym &s) const {
	Bwidth otherVal = bs->val();
	delete bs;
	
	if (op.div()) {
		checkDenom(otherVal);
		return new TimeSym(Time::Secd(theVal.byted()/BytesPerSec(otherVal)));
	}
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *SizeSym::operSN(const Oper &op, NumSym *ns, const SynSym &s) const {
	double otherFactor = ns->val();
	delete ns;
	
	if (op.div()) {
		checkDenom(otherFactor);
		return new SizeSym(theVal/otherFactor);
	}

	if (op.mult())
		return new SizeSym(theVal*otherFactor);

	return ExpressionSym::bnOper(op, s);
}

ostream &SizeSym::print(ostream &os, const String &) const {
	return os << theVal;
}
