
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglUndefOpExprSym.h"

const String UndefOpExprSym::TheType = "UndefOpExpr";


UndefOpExprSym::UndefOpExprSym(): ExpressionSym(TheType) {
}

bool UndefOpExprSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *UndefOpExprSym::dupe(const String &type) const {
	if (isA(type))
		return new UndefOpExprSym();
	else
		return ExpressionSym::dupe(type);
}
