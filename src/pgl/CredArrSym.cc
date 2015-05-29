
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/Rnd.h"
#include "base/StringArray.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/CredArrSym.h"



String CredArrSym::TheType = "CredArr";

static String strStringArr = "string[]";
static String strUser = "user";


CredArrSym::CredArrSym(): StringArrayBlockSym(TheType), theCreds(0) {
}

CredArrSym::~CredArrSym() {
	delete theCreds;
}

bool CredArrSym::isA(const String &type) const {
	return type == TheType || StringArrayBlockSym::isA(type);
}

SynSym *CredArrSym::dupe(const String &type) const {
	if (isA(type)) {
		CredArrSym *clone = new CredArrSym;
		clone->theGen = theGen; // preserve gen id
		return clone;
	} else
	if (type == strStringArr) {
		ArraySym *arr = new ArraySym(StringSym::TheType);
		arr->add(*this);
		return arr;
	} else {
		return 0;
	}
}

void CredArrSym::configure(int aCount, const String &aNamespace) {
	theGen.configure(aCount, aNamespace);
}

int CredArrSym::count() const {
	return theGen.count();
}

bool CredArrSym::probsSet() const {
	return false;
}

const SynSym *CredArrSym::itemProb(int idx, double &prob) const {
	Assert(theGen.count());

	delete theCreds;
	theCreds = new StringSym(theGen.item(idx));

	prob = 1.0/theGen.count();
	return theCreds;
}

const StringArrayBlock &CredArrSym::val() const {
	return theGen;
}

ostream &CredArrSym::print(ostream &os, const String &pfx) const {
	os << pfx << "credentials(" << theGen.count() << ", " << '"';
	if (theGen.space())
		os << theGen.space();
	os << '"' << ')';
	return os;
}
