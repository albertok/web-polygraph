
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PopModelSym.h"
#include "pgl/ClientBehaviorSym.h"


const String ClientBehaviorSym::TheType = "ClientBehavior";

static String strPop_model = "pop_model";
static String strRanges = "ranges";
static String strRangeArr = "Range[]";
static String strRecurrence = "recurrence";
static String strReq_methods = "req_methods";
static String strReq_types = "req_types";
static String strStringArr = "string[]";

ClientBehaviorSym::ClientBehaviorSym(): RobotSym(TheType) {
	theRec->reset();

	theRec->bAdd(PopModelSym::TheType, strPop_model, new PopModelSym);
	theRec->bAdd(strRangeArr, strRanges, 0);
	theRec->bAdd(NumSym::TheType, strRecurrence, 0);
	theRec->bAdd(strStringArr, strReq_methods, 0);
	theRec->bAdd(strStringArr, strReq_types, 0);
}

ClientBehaviorSym::ClientBehaviorSym(const String &aType, PglRec *aRec):
	RobotSym(aType, aRec) {
}

bool ClientBehaviorSym::isA(const String &type) const {
	return type == TheType || RobotSym::isA(type);
}

SynSym *ClientBehaviorSym::dupe(const String &type) const {
	if (type == RobotSym::TheType) {
		// cast ClientBehavior to Robot
		RobotSym *const r = new RobotSym();
		r->rec()->copyCommon(*theRec);
		return r;
	}
	if (isA(type))
		return new ClientBehaviorSym(this->type(), theRec->clone());
	return RobotSym::dupe(type);
}
