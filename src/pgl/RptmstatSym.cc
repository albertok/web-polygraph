
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/RptmstatSym.h"



const String RptmstatSym::TheType = "Rptmstat";

static String strLoad_delta = "load_delta";
static String strRptm_max = "rptm_max";
static String strRptm_min = "rptm_min";
static String strSample_dur = "sample_dur";


RptmstatSym::RptmstatSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(TimeSym::TheType, strSample_dur, 0);
	theRec->bAdd(NumSym::TheType, strLoad_delta, 0);
	theRec->bAdd(TimeSym::TheType, strRptm_min, 0);
	theRec->bAdd(TimeSym::TheType, strRptm_max, 0);
}

RptmstatSym::RptmstatSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool RptmstatSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *RptmstatSym::dupe(const String &type) const {
	if (isA(type))
		return new RptmstatSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

bool RptmstatSym::configured() const {
	double d = -1;
	return sampleDur() >= 0 || loadDelta(d) || 
		rptmMin() >= 0 || rptmMax() >= 0;
}

Time RptmstatSym::sampleDur() const {
	return getTime(strSample_dur);
}

bool RptmstatSym::loadDelta(double &loadDelta) const {
	return getDouble(strLoad_delta, loadDelta);
}

Time RptmstatSym::rptmMin() const {
	return getTime(strRptm_min);
}

Time RptmstatSym::rptmMax() const {
	return getTime(strRptm_max);
}
