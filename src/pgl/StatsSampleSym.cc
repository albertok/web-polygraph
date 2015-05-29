
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/StatsSampleSym.h"



const String StatsSampleSym::TheType = "StatsSample";

static String strCapacity = "capacity";
static String strName = "name";
static String strStart = "start";


StatsSampleSym::StatsSampleSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(StringSym::TheType, strName, 0);
	theRec->bAdd(TimeSym::TheType, strStart, 0);
	theRec->bAdd(IntSym::TheType, strCapacity, 0);
}

StatsSampleSym::StatsSampleSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool StatsSampleSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *StatsSampleSym::dupe(const String &type) const {
	if (isA(type))
		return new StatsSampleSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

String StatsSampleSym::name() const {
	return getString(strName);
}

Time StatsSampleSym::start() const {
	return getTime(strStart);
}

bool StatsSampleSym::capacity(int &cap) const {
	return getInt(strCapacity, cap);
}
