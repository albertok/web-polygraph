
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglBoolSym.h"
#include "pgl/PglRec.h"
#include "pgl/PglTimeSym.h"
#include "pgl/SocketSym.h"



String SocketSym::TheType = "Socket";

static String strLinger_tout = "linger_tout";
static String strNagle = "nagle";


SocketSym::SocketSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(BoolSym::TheType, strNagle, new BoolSym(false));
	theRec->bAdd(TimeSym::TheType, strLinger_tout, 0);
}

SocketSym::SocketSym(String const &aType, PglRec *aRec):
	RecSym(aType, aRec) {
}

bool SocketSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *SocketSym::dupe(const String &type) const {
	if (isA(type))
		return new SocketSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

bool SocketSym::lingerTout(Time &tout) const {
	SynSymTblItem *touti = 0;
	Assert(theRec->find(strLinger_tout, touti));
	if (touti->sym())
		tout = ((const TimeSym&)touti->sym()->cast(TimeSym::TheType)).val();
	return touti->sym() != 0;
}

bool SocketSym::nagle(bool &doNagle) const {
	SynSymTblItem *ni = 0;
	Assert(theRec->find(strNagle, ni));
	if (ni->sym())
		doNagle = ((const BoolSym&)ni->sym()->cast(BoolSym::TheType)).val();
	return ni->sym() != 0;
}
