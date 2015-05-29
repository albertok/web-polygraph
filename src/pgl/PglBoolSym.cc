
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglBoolSym.h"



String BoolSym::TheType = "bool";



BoolSym::BoolSym(bool aVal): ExpressionSym(TheType), theVal(aVal) {
}

bool BoolSym::isA(const String &type) const {
	return ExpressionSym::isA(type) ||
		type == TheType || type == TheType;
}

SynSym *BoolSym::dupe(const String &type) const {
	if (isA(type))
		return new BoolSym(theVal);
	return ExpressionSym::dupe(type);
}

ExpressionSym *BoolSym::unOper(const Oper &op) const {
	if (op.boolNot())
		return new BoolSym(!theVal);
	else
		return ExpressionSym::unOper(op);
}


ExpressionSym *BoolSym::bnOper(const Oper &op, const SynSym &s) const {
	bool otherVal = false;

	if (s.isA(BoolSym::TheType)) {
		otherVal = ((const BoolSym&)s.cast(BoolSym::TheType)).val();
	} else 
	if (BoolSym *is = (BoolSym*)s.clone(BoolSym::TheType)) {
		otherVal = is->val();
		delete is;
	} else
		return ExpressionSym::bnOper(op, s);

	if (op.same())
		return new BoolSym(theVal == otherVal);
	if (op.diff())
		return new BoolSym(theVal != otherVal);
	if (op.boolOr())
		return new BoolSym(theVal || otherVal);
	if (op.boolAnd())
		return new BoolSym(theVal && otherVal);
	if (op.boolXor())
		return new BoolSym(theVal ^ otherVal);

	return ExpressionSym::bnOper(op, s);
}

ostream &BoolSym::print(ostream &os, const String &) const {
	return os << (theVal ? "true" : "false");
}
