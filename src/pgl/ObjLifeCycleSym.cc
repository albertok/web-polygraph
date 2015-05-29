
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglDistrSym.h"
#include "pgl/ObjLifeCycleSym.h"



String ObjLifeCycleSym::TheType = "ObjLifeCycle";

static String strBirthday = "birthday";
static String strExpires = "expires";
static String strLength = "length";
static String strTimeDistr = "time_distr";
static String strTimeDistrArr = "time_distr[]";
static String strVariance = "variance";
static String strWithLmt = "with_lmt";


ObjLifeCycleSym::ObjLifeCycleSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(strTimeDistr, strBirthday, 0);
	theRec->bAdd(strTimeDistr, strLength, 0);
	theRec->bAdd(NumSym::TheType, strVariance, 0);
	theRec->bAdd(NumSym::TheType, strWithLmt, 0);
	theRec->bAdd(strTimeDistrArr, strExpires, 0);
}

ObjLifeCycleSym::ObjLifeCycleSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool ObjLifeCycleSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *ObjLifeCycleSym::dupe(const String &type) const {
	if (isA(type))
		return new ObjLifeCycleSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

RndDistr *ObjLifeCycleSym::bday() const {
	return getDistr(strBirthday);
}

RndDistr *ObjLifeCycleSym::length() const {
	return getDistr(strLength);
}

bool ObjLifeCycleSym::variance(double &ratio) const {
	return getDouble(strVariance, ratio);
}

bool ObjLifeCycleSym::withLmt(double &ratio) const {
	return getDouble(strWithLmt, ratio);
}

bool ObjLifeCycleSym::expires(Array<RndDistr*> &times, RndDistr *&selector, Array<QualifSym*> &qs) const {
	SynSymTblItem *ti = 0;
	Assert(theRec->find(strExpires, ti));
	if (!ti->sym())
		return false; // undefined

	ArraySym &a = (ArraySym&)ti->sym()->cast(ArraySym::TheType);
	selector = a.makeSelector(strExpires);
	for (int i = 0; i < a.count(); ++i) {
		DistrSym &ds = (DistrSym&)a[i]->cast(DistrSym::TheType);
		times.append(ds.val());
		qs.append(ds.qualifier());
	}

	return true;
}
