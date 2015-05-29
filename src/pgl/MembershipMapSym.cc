
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"
#include "pgl/MembershipMapSym.h"



const String MembershipMapSym::TheType = "MembershipMap";

static String strGroups = "group_space";
static String strMembers = "member_space";
static String strGroupsPerMember = "groups_per_member";
static String strStringArr = "string[]";
static String strInt_distr = "int_distr";

MembershipMapSym::MembershipMapSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(strStringArr, strGroups, 0);
	theRec->bAdd(strStringArr, strMembers, 0);
	theRec->bAdd(strInt_distr, strGroupsPerMember, 0);
}

MembershipMapSym::MembershipMapSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool MembershipMapSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *MembershipMapSym::dupe(const String &type) const {
	if (isA(type))
		return new MembershipMapSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

bool MembershipMapSym::groupSpace(StringArray &names) const {
	return getStringArray(strGroups, names);
}

bool MembershipMapSym::memberSpace(StringArray &names) const {
	return getStringArray(strMembers, names);
}

RndDistr *MembershipMapSym::groupsPerMember() const {
	return getDistr(strGroupsPerMember);
}
