
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/BigSize.h"
#include "pgl/PglRec.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/DutStateSym.h"



const String DutStateSym::TheType = "DutState";

static String strRptm_min = "rptm_min";
static String strRptm_max = "rptm_max";
static String strFillSize_min = "fill_size_min";
static String strFillSize_max = "fill_size_max";
static String strXactions_min = "xactions_min";
static String strXactions_max = "xactions_max";
static String strRepRate_min = "rep_rate_min";
static String strRepRate_max = "rep_rate_max";
static String strErrors_min = "errors_min";
static String strErrors_max = "errors_max";
static String strErrorRatio_min = "error_ratio_min";
static String strErrorRatio_max = "error_ratio_max";
static String strDhr_min = "dhr_min";
static String strDhr_max = "dhr_max";

DutStateSym::DutStateSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(TimeSym::TheType, strRptm_min, 0);
	theRec->bAdd(TimeSym::TheType, strRptm_max, 0);
	theRec->bAdd(SizeSym::TheType, strFillSize_min, 0);
	theRec->bAdd(SizeSym::TheType, strFillSize_max, 0);
	theRec->bAdd(IntSym::TheType, strXactions_min, 0);
	theRec->bAdd(IntSym::TheType, strXactions_max, 0);
	theRec->bAdd(NumSym::TheType, strRepRate_min, 0);
	theRec->bAdd(NumSym::TheType, strRepRate_max, 0);
	theRec->bAdd(IntSym::TheType, strErrors_min, 0);
	theRec->bAdd(IntSym::TheType, strErrors_max, 0);
	theRec->bAdd(NumSym::TheType, strErrorRatio_min, 0);
	theRec->bAdd(NumSym::TheType, strErrorRatio_max, 0);
	theRec->bAdd(NumSym::TheType, strDhr_min, 0);
	theRec->bAdd(NumSym::TheType, strDhr_max, 0);
}

DutStateSym::DutStateSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool DutStateSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *DutStateSym::dupe(const String &type) const {
	if (isA(type))
		return new DutStateSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

Time DutStateSym::rptmMin() const {
	return getTime(strRptm_min);
}

Time DutStateSym::rptmMax() const {
	return getTime(strRptm_max);
}

bool DutStateSym::fillSizeMin(BigSize &size) const {
	return getSize(strFillSize_min, size);
}

bool DutStateSym::fillSizeMax(BigSize &size) const {
	return getSize(strFillSize_max, size);
}

bool DutStateSym::xactionsMin(int &val) const {
	return getInt(strXactions_min, val);
}

bool DutStateSym::xactionsMax(int &val) const {
	return getInt(strXactions_max, val);
}

bool DutStateSym::repRateMin(double &val) const {
	return getDouble(strRepRate_min, val);
}

bool DutStateSym::repRateMax(double &val) const {
	return getDouble(strRepRate_max, val);
}

bool DutStateSym::errorsMin(int &val) const {
	return getInt(strErrors_min, val);
}

bool DutStateSym::errorsMax(int &val) const {
	return getInt(strErrors_max, val);
}

bool DutStateSym::errorRatioMin(double &val) const {
	return getDouble(strErrorRatio_min, val);
}

bool DutStateSym::errorRatioMax(double &val) const {
	return getDouble(strErrorRatio_max, val);
}

bool DutStateSym::dhrMin(double &val) const {
	return getDouble(strDhr_min, val);
}

bool DutStateSym::dhrMax(double &val) const {
	return getDouble(strDhr_max, val);
}

