
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "pgl/pgl.h"

#include "xstd/String.h"
#include "xstd/BigSize.h"
#include "pgl/PglRec.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/GoalSym.h"



String GoalSym::TheType = "Goal";

static String strDuration = "duration";
static String strErrors = "errors";
static String strFill_size = "fill_size";
static String strXactions = "xactions";


GoalSym::GoalSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(TimeSym::TheType, strDuration, 0);
	theRec->bAdd(IntSym::TheType, strXactions, 0);
	theRec->bAdd(SizeSym::TheType, strFill_size, 0);
	theRec->bAdd(NumSym::TheType, strErrors, 0);
}

GoalSym::GoalSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool GoalSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *GoalSym::dupe(const String &type) const {
	if (isA(type))
		return new GoalSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

Time GoalSym::duration() const {
	return getTime(strDuration);
}

bool GoalSym::fillSize(BigSize &sz) const {
	SynSymTblItem *szi = 0;
	Assert(theRec);
	Assert(theRec->find(strFill_size, szi));
	if (szi->sym())
		sz = ((const SizeSym&)szi->sym()->cast(SizeSym::TheType)).val();
	return szi->sym() != 0;
}

bool GoalSym::xactCount(Counter &count) const {
	SynSymTblItem *xi = 0;
	Assert(theRec);
	Assert(theRec->find(strXactions, xi));
	if (xi->sym())
		count = ((const IntSym&)xi->sym()->cast(IntSym::TheType)).val();
	return xi->sym() != 0;
}

bool GoalSym::errs(double &ratio, Counter &count) const {
	SynSymTblItem *ei = 0;
	Assert(theRec);
	Assert(theRec->find(strErrors, ei));
	if (ei->sym()) {
		double level = ((const NumSym&)ei->sym()->cast(NumSym::TheType)).val();
		if (level < 1)
			ratio = level;
		else
			count = level;
	}
	return ei->sym() != 0;
}

