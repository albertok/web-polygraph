
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglRegExSym.h"
#include "pgl/AclSym.h"


String AclSym::TheType = "Acl";

static String strAllow = "allow";
static String strDeny = "deny";
static String strRewrite = "rewrite";
static String strCheckDomestic = "check_domestic";
static String strCheckForeign = "check_foreign";


AclSym::AclSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(RegExSym::TheType, strAllow, 0);
	theRec->bAdd(RegExSym::TheType, strDeny, 0);
	theRec->bAdd(RegExSym::TheType, strRewrite, 0);
	theRec->bAdd(NumSym::TheType, strCheckDomestic, 0);
	theRec->bAdd(NumSym::TheType, strCheckForeign, 0);
}

AclSym::AclSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool AclSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *AclSym::dupe(const String &type) const {
	if (isA(type))
		return new AclSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

RegExExpr *AclSym::allow() const {
	return getGroup(strAllow);
}

RegExExpr *AclSym::deny() const {
	return getGroup(strDeny);
}

RegExExpr *AclSym::rewrite() const {
	return getGroup(strRewrite);
}

bool AclSym::checkDomestic(double &prob) const {
	return getDouble(strCheckDomestic, prob);
}

bool AclSym::checkForeign(double &prob) const {
	return getDouble(strCheckForeign, prob);
}

RegExExpr *AclSym::getGroup(const String &name) const {
	SynSymTblItem *gi = 0;
	Assert(theRec->find(name, gi));
	return gi->sym() ?
		((RegExSym&)gi->sym()->cast(RegExSym::TheType)).val() : 0;
}
