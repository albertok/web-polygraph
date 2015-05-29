
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/PglNumSym.h"
#include "pgl/DynamicNameSym.h"


String DynamicNameSym::TheType = "DynamicName";

static String strNewProb = "new_prob";


DynamicNameSym::DynamicNameSym() {
	theRec->bAdd(NumSym::TheType, strNewProb, 0);
}

DynamicNameSym::DynamicNameSym(const String &aType, PglRec *aRec): NetAddrSym(aType, aRec) {
}

bool DynamicNameSym::isA(const String &type) const {
	return NetAddrSym::isA(type) || type == TheType;
}

SynSym *DynamicNameSym::dupe(const String &type) const {
	if (isA(type))
		return new DynamicNameSym(this->type(), theRec->clone());
	return NetAddrSym::dupe(type);
}

void DynamicNameSym::val(const NetAddr &addr) {
	if (addr.addrA().startsWith("*."))
		NetAddrSym::val(NetAddr(addr.addrA()(1, String::npos), addr.port()));
	else
	if (addr.addrA().startsWith("."))
		NetAddrSym::val(addr);
	else
		NetAddrSym::val(NetAddr("." + addr.addrA(), addr.port()));
}

void DynamicNameSym::setNewProb(const double  prob) {
	SynSymTblItem *sym = 0;
	Assert(theRec->find(strNewProb, sym));
	delete sym->sym();
	sym->sym(new NumSym(prob));
}

bool DynamicNameSym::newProb(double &prob) const {
	return getDouble(strNewProb, prob);
}
