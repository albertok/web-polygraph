
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/ContentSym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglRec.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglStringSym.h"
#include "pgl/ServerSym.h"



const String ServerSym::TheType = "Server";

static String strContentArr = "Content[]";
static String strAccept_lmt = "accept_lmt";
static String strContents = "contents";
static String strDirect_access = "direct_access";
static String strRep_types = "rep_types";
static String strSize_distr = "size_distr";
static String strFloat_distr = "float_distr";
static String strStringArr = "string[]";
static String strCookie_set_count = "cookie_set_count";
static String strCookie_set_prob = "cookie_set_prob";
static String strCookie_value_size = "cookie_value_size";
static String strReqBodyAllowed = "req_body_allowed";
static String strProtocol = "protocol";


ServerSym::ServerSym(): AgentSym(TheType) {
	theRec->bAdd(IntSym::TheType, strAccept_lmt, 0);
	theRec->bAdd(strContentArr, strContents, 0);
	theRec->bAdd(strContentArr, strDirect_access, 0);
	theRec->bAdd(strStringArr, strRep_types, 0);
	theRec->bAdd(strFloat_distr, strCookie_set_count, 0);
	theRec->bAdd(NumSym::TheType, strCookie_set_prob, 0);
	theRec->bAdd(strSize_distr, strCookie_value_size, 0);
	theRec->bAdd(NumSym::TheType, strReqBodyAllowed, 0);
	theRec->bAdd(strStringArr, strProtocol, 0);
}

ServerSym::ServerSym(const String aType, PglRec *aRec):	AgentSym(aType, aRec) {
}

bool ServerSym::isA(const String &type) const {
	return type == TheType || AgentSym::isA(type);
}

SynSym *ServerSym::dupe(const String &type) const {
	if (isA(type))
		return new ServerSym(this->type(), theRec->clone());
	return AgentSym::dupe(type);
}

bool ServerSym::acceptLmt(int &lmt) const {
	return getInt(strAccept_lmt, lmt);
}

bool ServerSym::contents(Array<ContentSym*> &ccfgs, Array<double> &cprobs) const {
	SynSymTblItem *conti = 0;
	Assert(theRec->find(strContents, conti));
	if (!conti->sym())
		return false; // undefined

	ArraySym &a = (ArraySym&)conti->sym()->cast(ArraySym::TheType);
	a.copyProbs(cprobs);
	a.exportA(ccfgs);
	return true;
}

bool ServerSym::directAccess(Array<ContentSym*> &dacfgs, Array<double> &daprobs) const {
	SynSymTblItem *dai = 0;
	Assert(theRec->find(strDirect_access, dai));

	const bool hasDa = dai->sym() != 0;

	// use contents array if direct_access is not defined
	if (!hasDa)
		Assert(theRec->find(strContents, dai));
	if (!dai->sym())
		return false; // undefined

	ArraySym &a = (ArraySym&)dai->sym()->cast(ArraySym::TheType);
	a.exportA(dacfgs);

	daprobs.reset();
	if (hasDa && a.probsSet())
		a.copyProbs(daprobs);

	return true;
}

String ServerSym::msgTypesField() const {
	return strRep_types;
}

bool ServerSym::cookieSetProb(double &prob) const {
	return getDouble(strCookie_set_prob, prob);
}

RndDistr *ServerSym::cookieSetCount() const {
	return getDistr(strCookie_set_count);
}

RndDistr *ServerSym::cookieValueSize() const {
	return getDistr(strCookie_value_size);
}

bool ServerSym::reqBodyAllowed(double &f) const {
	return getDouble(strReqBodyAllowed, f);
}

RndDistr *ServerSym::protocols(const TokenIdentifier &protocolNames) const {
	return namesToDistr(strProtocol, protocolNames);
}
