
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglBoolSym.h"
#include "pgl/DumperSym.h"



String DumperSym::TheType = "Dumper";

static String strRep_hdrs = "rep_hdrs";
static String strReq_hdrs = "req_hdrs";


DumperSym::DumperSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(BoolSym::TheType, strReq_hdrs, 0);
	theRec->bAdd(BoolSym::TheType, strRep_hdrs, 0);
}

DumperSym::DumperSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool DumperSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *DumperSym::dupe(const String &type) const {
	if (isA(type))
		return new DumperSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}
