
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "pgl/PglRegExSym.h"
#include "client/RegExGroups.h"


RegExGroup *RegExGroup::FromExpr(RegExExpr *expr) {
	if (expr) {
		switch (expr->theOper) {
			case RegExExpr::opNone:
				return new RegExGrpOne(expr->theVal);
			case RegExExpr::opNot:
				return new RegExGrpNot(FromExpr(expr->theLhs));
			case RegExExpr::opAnd: {
				RegExGrpAnd *res = new RegExGrpAnd;
				res->add(FromExpr(expr->theLhs));
				res->add(FromExpr(expr->theRhs));
				return res;
			}
			case RegExExpr::opOr: {
				RegExGrpOr *res = new RegExGrpOr;
				res->add(FromExpr(expr->theLhs));
				res->add(FromExpr(expr->theRhs));
				return res;
			}
			default: {
				Assert(false);
				return 0;
			}
		}
	}
	return 0;
}
