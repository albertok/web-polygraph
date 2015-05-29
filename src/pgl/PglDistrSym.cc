
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/Assert.h"
#include "xstd/Size.h"
#include "xstd/Time.h"
#include "xstd/String.h"
#include "xstd/Rnd.h"
#include "pgl/PglQualifSym.h"
#include "pgl/PglDistrSym.h"



String DistrSym::TheType = "Distr";

static String strInt_distr = "int_distr";
static String strSize_distr = "size_distr";
static String strTime_distr = "time_distr";


// XXX: we are leaking actual distributions as they are never deleted!

DistrSym::DistrSym(const String &aType, RndDistr *aVal): 
	ExpressionSym(aType), theVal(aVal), theQual(0) {
}

DistrSym::~DistrSym() {
	delete theQual;
}

bool DistrSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *DistrSym::dupe(const String &type) const {
	if (isA(type)) {
		DistrSym *d = new DistrSym(this->type(), theVal);
		d->qualifier(theQual, theQfOp);
		return d;
	}
	return ExpressionSym::dupe(type);
}

void DistrSym::qualifier(const QualifSym *aQual, const String &anOp) {
	delete theQual;
	theQual = aQual ? (QualifSym*)aQual->clone() : 0;
	theQfOp = anOp;
}

ostream &DistrSym::print(ostream &os, const String &) const {
	if (theQual)
		theQual->print(os, String()) << theQfOp;

	if (theVal) {
		if (type() == strInt_distr)
			theVal->print(os, &IntArgPrinter);
		else
		if (type() == strSize_distr)
			theVal->print(os, &SizeArgPrinter);
		else
		if (type() == strTime_distr)
			theVal->print(os, &TimeArgPrinter);
		else
			theVal->print(os);
	} else
		os << "<none>";
	return os;
}

void DistrSym::IntArgPrinter(ostream &os, double arg, int) {
	os << (int)arg;
}

void DistrSym::SizeArgPrinter(ostream &os, double arg, int) {
	os << Size((int)arg);
}

void DistrSym::TimeArgPrinter(ostream &os, double arg, int) {
	os << Time::Secd(arg);
}
