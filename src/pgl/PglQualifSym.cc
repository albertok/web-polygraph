
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/Assert.h"
#include "xstd/String.h"
#include "xstd/Rnd.h"
#include "pgl/PglDistrSym.h"
#include "pgl/PglQualifSym.h"



const String QualifSym::TheType = "Qualif";

static String strLmt = "lmt";
static String strNmt = "nmt";
static String strNow = "now";
static String strQualifier = "qualifier";
static String strTime_distr = "time_distr";


QualifSym::QualifSym(Kind aKind): ExpressionSym(strQualifier), theKind(aKind) {
}

bool QualifSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *QualifSym::dupe(const String &type) const {
	if (isA(type))
		return new QualifSym(theKind);
	return ExpressionSym::dupe(type);
}

// lmt + expr  or  now + expr
ExpressionSym *QualifSym::bnOper(const Oper &op, const SynSym &exp) const {
	if (!op.plus())
		return ExpressionSym::bnOper(op, exp);

	Assert(theKind != qfNone);
	DistrSym *d = (DistrSym*)exp.clone(strTime_distr);
	if (!d)
		cerr << exp.loc() << "`time_distr' expected after a qualifier; got `" << exp.type() << "'" << endl << xexit;
	d->qualifier(this, op.image());
	return d;
}

ostream &QualifSym::print(ostream &os, const String &) const {
	if (theKind == qfLmt)
		os << strLmt;
	else
	if (theKind == qfNow)
		os << strNow;
	else
	if (theKind == qfNmt)
		os << strNmt;
	return os;
}
