
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"

#include "pgl/PglExprSym.h"



String ExpressionSym::TheType = "Expression";


bool ExpressionSym::isA(const String &type) const {
	return SynSym::isA(type) || type == TheType;
}

ExpressionSym *ExpressionSym::unOper(const Oper &op) const {
	cerr << theLoc << "type `" << type() 
		<< "' has no unary operator `" << op.image() << "'" << endl << xexit;
	return 0;
}


ExpressionSym *ExpressionSym::bnOper(const Oper &op, const SynSym &exp) const {
	cerr << theLoc << "binary operator `" << op.image() << 
		"' cannot be applied to types `" << type() <<
		"' and `" << exp.type() << "'" << endl << xexit;
	return 0;
}

void ExpressionSym::checkDenom(int val) const {
	if (val == 0)
		cerr << theLoc << "denominator is zero" << endl << xexit;
}

void ExpressionSym::checkDenom(double val) const {
	if (fabs(val) < 1e-8)
		cerr << theLoc << "denominator is "
			<< (val ? "[virtually]" : "")
			<< " zero" << endl << xexit;
}
