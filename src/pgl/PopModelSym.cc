
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglPopDistrSym.h"
#include "pgl/PopModelSym.h"



String PopModelSym::TheType = "PopModel";

static String strBhr_discrimination = "bhr_discrimination";
static String strHot_set_frac = "hot_set_frac";
static String strHot_set_prob = "hot_set_prob";
static String strPop_distr = "pop_distr";


PopModelSym::PopModelSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(PopDistrSym::TheType, strPop_distr, 0);
	theRec->bAdd(NumSym::TheType, strHot_set_frac, 0);
	theRec->bAdd(NumSym::TheType, strHot_set_prob, 0);
	theRec->bAdd(NumSym::TheType, strBhr_discrimination, 0);
}

PopModelSym::PopModelSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool PopModelSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *PopModelSym::dupe(const String &type) const {
	if (isA(type))
		return new PopModelSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

PopDistr *PopModelSym::popDistr() const {
	SynSymTblItem *pdi = 0;
	Assert(theRec->find(strPop_distr, pdi));
	return pdi->sym() ?
		((const PopDistrSym&)pdi->sym()->cast(PopDistrSym::TheType)).val() : 0;
}

bool PopModelSym::hotSetFrac(double &ratio) const {
	return getDouble(strHot_set_frac, ratio);
}

bool PopModelSym::hotSetProb(double &prob) const {
	return getDouble(strHot_set_prob, prob);
}

bool PopModelSym::bhrDiscr(double &prob) const {
	return getDouble(strBhr_discrimination, prob);
}
