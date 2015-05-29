
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglBoolSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglBwidthSym.h"
#include "pgl/PglRateSym.h"

#include "xstd/gadgets.h"


String RateSym::TheType = "rate";



RateSym::RateSym(double aCount, Time anInterval): ExpressionSym(TheType),
	theCount(aCount), theInterval(anInterval) {
}

RateSym::RateSym(double aVal): ExpressionSym(TheType) {
	val(aVal);
}

bool RateSym::isA(const String &type) const {
	return ExpressionSym::isA(type) ||
		type == TheType || type == TheType;
}

SynSym *RateSym::dupe(const String &type) const {
	if (isA(type))
		return new RateSym(theCount, theInterval);
	return ExpressionSym::dupe(type);
}

double RateSym::val() const {
	return Ratio(theCount, theInterval.secd());
}

void RateSym::val(double aVal) {
	theCount = aVal;
	theInterval = Time::Sec(1);
}

ExpressionSym *RateSym::unOper(const Oper &op) const {
	if (op.plus())
		return new RateSym(+theCount, theInterval);
	if (op.minus())
		return new RateSym(-theCount, theInterval);
	return ExpressionSym::unOper(op);
}

ExpressionSym *RateSym::bnOper(const Oper &op, const SynSym &s) const {
	if (RateSym *rs = (RateSym*)s.clone(TheType))
		return operRR(op, rs, s);

	if (TimeSym *ts = (TimeSym*)s.clone(TimeSym::TheType))
		return operRT(op, ts, s);

	if (SizeSym *ss = (SizeSym*)s.clone(SizeSym::TheType))
		return operRS(op, ss, s);

	if (NumSym *ns = (NumSym*)s.clone(NumSym::TheType))
		return operRN(op, ns, s);

	return ExpressionSym::bnOper(op, s);
}

// rate op rate
ExpressionSym *RateSym::operRR(const Oper &op, RateSym *rs, const SynSym &s) const {
	double otherVal = rs->val();
	delete rs;

	if (op.div()) {
		checkDenom(otherVal);
		return new NumSym(val() / otherVal);
	}

	if (op.lessTrue())
		return new BoolSym(val() < otherVal);
	if (op.lessOrEq())
		return new BoolSym(val() <= otherVal);
	if (op.greaterTrue())
		return new BoolSym(val() > otherVal);
	if (op.greaterOrEq())
		return new BoolSym(val() >= otherVal);
	if (op.plus())
		return new RateSym(val() + otherVal);
	if (op.minus())
		return new RateSym(val() - otherVal);
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *RateSym::operRS(const Oper &op, SizeSym *szs, const SynSym &s) const {
	BigSize sz = szs->val();
	delete szs;

	if (op.mult())
		return new BwidthSym(Bandwidth(sz*theCount, theInterval));
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *RateSym::operRT(const Oper &op, TimeSym *ts, const SynSym &s) const {
	Time otherInterval = ts->val();
	delete ts;

	if (op.mult())
		return new NumSym(theCount*(otherInterval/theInterval));
	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *RateSym::operRN(const Oper &op, NumSym *ns, const SynSym &s) const {
	double otherCount = ns->val();
	delete ns;
	
	if (op.div()) {
		checkDenom(otherCount);
		return new RateSym(theCount/otherCount, theInterval);
	}

	if (op.mult())
		return new RateSym(theCount*otherCount, theInterval);
	return ExpressionSym::bnOper(op, s);
}

ostream &RateSym::print(ostream &os, const String &) const {
	return os << val() << "/sec";
}
