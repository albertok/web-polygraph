
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/PglCodeSym.h"
#include "pgl/PglBoolSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/GoalSym.h"
#include "pgl/RptmstatSym.h"
#include "pgl/StatsSampleSym.h"
#include "pgl/PhaseSym.h"



const String PhaseSym::TheType = "Phase";

static String strStatsSampleArr = "StatsSample[]";
static String strGoal = "goal";
static String strLoad_factor_beg = "load_factor_beg";
static String strLoad_factor_end = "load_factor_end";
static String strLog_stats = "log_stats";
static String strName = "name";
static String strPopulus_factor_beg = "populus_factor_beg";
static String strPopulus_factor_end = "populus_factor_end";
static String strPrimary = "primary";
static String strRecur_factor_beg = "recur_factor_beg";
static String strRecur_factor_end = "recur_factor_end";
static String strRptmstat = "rptmstat";
static String strScript = "script";
static String strSpecial_msg_factor_beg = "special_msg_factor_beg";
static String strSpecial_msg_factor_end = "special_msg_factor_end";
static String strStats_samples = "stats_samples";
static String strSynchronize = "synchronize";
static String strWait_wss_freeze = "wait_wss_freeze";


PhaseSym::PhaseSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(StringSym::TheType, strName, 0);
	theRec->bAdd(GoalSym::TheType, strGoal, new GoalSym);
	theRec->bAdd(RptmstatSym::TheType, strRptmstat, new RptmstatSym);
	theRec->bAdd(CodeSym::TheType, strScript, 0);

	theRec->bAdd(NumSym::TheType, strPopulus_factor_beg, 0);
	theRec->bAdd(NumSym::TheType, strPopulus_factor_end, 0);
	theRec->bAdd(NumSym::TheType, strLoad_factor_beg, 0);
	theRec->bAdd(NumSym::TheType, strLoad_factor_end, 0);
	theRec->bAdd(NumSym::TheType, strRecur_factor_beg, 0);
	theRec->bAdd(NumSym::TheType, strRecur_factor_end, 0);
	theRec->bAdd(NumSym::TheType, strSpecial_msg_factor_beg, 0);
	theRec->bAdd(NumSym::TheType, strSpecial_msg_factor_end, 0);

	theRec->bAdd(strStatsSampleArr, strStats_samples, 0);
	theRec->bAdd(BoolSym::TheType, strLog_stats, 0);
	theRec->bAdd(BoolSym::TheType, strPrimary, 0);

	theRec->bAdd(BoolSym::TheType, strWait_wss_freeze, 0);
	theRec->bAdd(BoolSym::TheType, strSynchronize, 0);
}

PhaseSym::PhaseSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool PhaseSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *PhaseSym::dupe(const String &type) const {
	if (isA(type))
		return new PhaseSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

void PhaseSym::name(const String &aName) {
	SynSymTblItem *ni = 0;
	Assert(theRec->find(strName, ni));
	delete ni->sym();
	ni->sym(new StringSym(aName));
}

String PhaseSym::name() const {
	return getString(strName);
}

GoalSym *PhaseSym::goal() const {
	SynSymTblItem *gi = 0;
	Assert(theRec->find(strGoal, gi));
	Assert(gi->sym());
	return &(GoalSym&)gi->sym()->cast(GoalSym::TheType);
}

RptmstatSym *PhaseSym::rptmstat() const {
	SynSymTblItem *gi = 0;
	Assert(theRec->find(strRptmstat, gi));
	Assert(gi->sym());
	RptmstatSym *s = &(RptmstatSym&)gi->sym()->cast(RptmstatSym::TheType);
	return s->configured() ? s : 0;
}

CodeSym *PhaseSym::script() const {
	SynSymTblItem *si = 0;
	Assert(theRec->find(strScript, si));
	if (!si->sym())
		return 0;
	return &(CodeSym&)si->sym()->cast(CodeSym::TheType);
}

bool PhaseSym::populusFactorBeg(double &lf) const {
	return getDouble(strPopulus_factor_beg, lf);
}

bool PhaseSym::populusFactorEnd(double &lf) const {
	return getDouble(strPopulus_factor_end, lf);
}

bool PhaseSym::loadFactorBeg(double &lf) const {
	return getDouble(strLoad_factor_beg, lf);
}

bool PhaseSym::loadFactorEnd(double &lf) const {
	return getDouble(strLoad_factor_end, lf);
}

bool PhaseSym::recurFactorBeg(double &rf) const {
	return getDouble(strRecur_factor_beg, rf);
}

bool PhaseSym::recurFactorEnd(double &rf) const {
	return getDouble(strRecur_factor_end, rf);
}

bool PhaseSym::specialMsgFactorBeg(double &sf) const {
	return getDouble(strSpecial_msg_factor_beg, sf);
}

bool PhaseSym::specialMsgFactorEnd(double &sf) const {
	return getDouble(strSpecial_msg_factor_end, sf);
}

bool PhaseSym::logStats(bool &ls) const {
	return getBool(strLog_stats, ls);
}

bool PhaseSym::primary(bool &ls) const {
	return getBool(strPrimary, ls);
}

bool PhaseSym::waitWssFreeze(bool &wwf) const {
	return getBool(strWait_wss_freeze, wwf);
}

bool PhaseSym::synchronize(bool &s) const {
	return getBool(strSynchronize, s);
}

bool PhaseSym::statsSamples(Array<const StatsSampleSym*> &samples) const {
	SynSymTblItem *ssi = 0;
	Assert(theRec->find(strStats_samples, ssi));
	if (!ssi->sym())
		return false; // undefined

	// build address array and selector
	ArraySym &ssa = (ArraySym&)ssi->sym()->cast(ArraySym::TheType);
	samples.stretch(ssa.count());
	for (int i = 0; i < ssa.count(); ++i) {
		const StatsSampleSym * ss =
			&((const StatsSampleSym&)ssa[i]->cast(StatsSampleSym::TheType));
		samples.append(ss);
	}

	return samples.count() != 0;
}
