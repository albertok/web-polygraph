
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xparser/TokenSym.h"
#include "pgl/PglParser.h"
#include "pgl/PglExprOper.h"


ExpressionOper ExpressionOper::ToInt() {
	const TokenSym t("int", INT_TOKEN);
	return ExpressionOper(t);
}

ExpressionOper ExpressionOper::Same() {
	const TokenSym t("==", EQUAL_TOKEN);
	return ExpressionOper(t);
}

ExpressionOper::ExpressionOper(const TokenSym &t): theImage(t.spelling()),
	theId(t.id()) {
}

const String &ExpressionOper::image() const {
	return theImage;
}

bool ExpressionOper::boolNot() const {
	return theId == NOT_TOKEN;
}

bool ExpressionOper::boolAnd() const {
	return theId == BOOL_AND_TOKEN;
}

bool ExpressionOper::boolOr() const {
	return theId == BOOL_OR_TOKEN;
}

bool ExpressionOper::boolXor() const {
	return theId == BOOL_XOR_TOKEN;
}

bool ExpressionOper::same() const {
	return theId == EQUAL_TOKEN;
}

bool ExpressionOper::diff() const {
	return theId == NEQUAL_TOKEN;
}

bool ExpressionOper::lessOrEq() const {
	return theId == LTE_TOKEN;
}

bool ExpressionOper::lessTrue() const {
	return theId == LT_TOKEN;
}

bool ExpressionOper::greaterOrEq() const {
	return theId == GTE_TOKEN;
}

bool ExpressionOper::greaterTrue() const {
	return theId == GT_TOKEN;
}

bool ExpressionOper::plus() const {
	return theId == PLUS_TOKEN;
}

bool ExpressionOper::minus() const {
	return theId == MINUS_TOKEN;
}

bool ExpressionOper::mult() const {
	return theId == MUL_TOKEN;
}

bool ExpressionOper::div() const {
	return theId == DIV_TOKEN;
}

bool ExpressionOper::mod() const {
	return theId == MOD_TOKEN;
}

bool ExpressionOper::power() const {
	return theId == POWER_TOKEN;
}

bool ExpressionOper::range() const {
	return theId == THRU_TOKEN;
}

bool ExpressionOper::clone() const {
	return theId == CLONE_TOKEN;
}
