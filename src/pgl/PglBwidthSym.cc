
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglBoolSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglBwidthSym.h"

#include "xstd/gadgets.h"


String BwidthSym::TheType = "bwidth";



static BigSize bytes2size(double bytes) { return BigSize::MBd(bytes/1024/1024); }


BwidthSym::BwidthSym(BigSize aSize, Time anInterval): ExpressionSym(TheType),
	theVal(::Bandwidth(aSize, anInterval)) {
}

BwidthSym::BwidthSym(double aVal): ExpressionSym(TheType) {
	val(aVal);
}

bool BwidthSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *BwidthSym::dupe(const String &type) const {
	if (isA(type))
		return new BwidthSym(theVal);
	return ExpressionSym::dupe(type);
}

Bwidth BwidthSym::val() const {
	return theVal;
}

void BwidthSym::val(Bwidth aVal) {
	theVal = aVal;
}

ExpressionSym *BwidthSym::unOper(const Oper &op) const {
	if (op.plus())
		return new BwidthSym(+theVal);
	else
	if (op.minus())
		return new BwidthSym(-theVal);
	else
		return ExpressionSym::unOper(op);
}

ExpressionSym *BwidthSym::bnOper(const Oper &op, const SynSym &s) const {
	if (BwidthSym *rs = (BwidthSym*)s.clone(BwidthSym::TheType))
		return operBB(op, rs, s);

	if (TimeSym *ts = (TimeSym*)s.clone(TimeSym::TheType))
		return operBT(op, ts, s);

	if (NumSym *ns = (NumSym*)s.clone(NumSym::TheType))
		return operBN(op, ns, s);

	return ExpressionSym::bnOper(op, s);
}

// bwidth op bwidth
ExpressionSym *BwidthSym::operBB(const Oper &op, BwidthSym *rs, const SynSym &s) const {
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
		return new BwidthSym(val() + otherVal);
	if (op.minus())
		return new BwidthSym(val() - otherVal);

	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *BwidthSym::operBT(const Oper &op, TimeSym *ts, const SynSym &s) const {
	Time otherInterval = ts->val();
	delete ts;

	if (op.mult())
		return new SizeSym(bytes2size(BytesPerSec(theVal)*otherInterval.secd()));

	return ExpressionSym::bnOper(op, s);
}

ExpressionSym *BwidthSym::operBN(const Oper &op, NumSym *ns, const SynSym &s) const {
	double otherFactor = ns->val();
	delete ns;
	
	if (op.div()) {
		checkDenom(otherFactor);
		return new BwidthSym(theVal/otherFactor);
	}

	if (op.mult())
		return new BwidthSym(theVal*otherFactor);

	return ExpressionSym::bnOper(op, s);
}

ostream &BwidthSym::print(ostream &os, const String &) const {
	return os << bytes2size(theVal*1) << "/sec";
}


Bwidth Bandwidth(BigSize sz, Time tm) {
	return sz.byted()/tm.secd();
}
