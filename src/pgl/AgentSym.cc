
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglDistrSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/SocketSym.h"
#include "pgl/UniqIdSym.h"
#include "pgl/PopModelSym.h"
#include "pgl/AgentSym.h"
#include "pgl/MimeHeaderSym.h"



String AgentSym::TheType = "Agent";

static String strStringArr = "string[]";
static String strAbort_prob = "abort_prob";
static String strCookie_sender = "cookie_sender";
static String strCustomStatsScope = "custom_stats_scope";
static String strIdle_pconn_tout = "idle_pconn_tout";
static String strInt_distr = "int_distr";
static String strKind = "kind";
static String strHttp_versions = "http_versions";
static String strPconn_use_lmt = "pconn_use_lmt";
static String strPop_model = "pop_model";
static String strSocket = "socket";
static String strTime_distr = "time_distr";
static String strWorld = "world";
static String strXact_lifetime = "xact_lifetime";
static String strXact_think = "xact_think";
static String strHttpHeaders = "http_headers";
static String strMimeHeaderArr = "MimeHeader[]";


AgentSym::AgentSym(const String &aType): HostsBasedSym(aType) {
	theRec->bAdd(SocketSym::TheType, strSocket, new SocketSym);

	theRec->bAdd(StringSym::TheType, strKind, 0);
	theRec->bAdd(UniqIdSym::TheType, strWorld, 0);
	theRec->bAdd(strStringArr, strHttp_versions, 0);
	theRec->bAdd(strTime_distr, strXact_lifetime, 0);
	theRec->bAdd(strTime_distr, strXact_think, 0);
	theRec->bAdd(strInt_distr, strPconn_use_lmt, 0);
	theRec->bAdd(TimeSym::TheType, strIdle_pconn_tout, 0);
	theRec->bAdd(PopModelSym::TheType, strPop_model, new PopModelSym);
	theRec->bAdd(NumSym::TheType, strAbort_prob, 0);
	theRec->bAdd(NumSym::TheType, strCookie_sender, 0);
	theRec->bAdd(strStringArr, strCustomStatsScope, 0);
	theRec->bAdd(strMimeHeaderArr, strHttpHeaders, 0);
}

AgentSym::AgentSym(const String &aType, PglRec *aRec): HostsBasedSym(aType, aRec) {
}

bool AgentSym::isA(const String &type) const {
	return HostsBasedSym::isA(type) || type == TheType;
}

String AgentSym::kind() const {
	return getString(strKind);
}

UniqId AgentSym::world() const {
	SynSymTblItem *wi = 0;
	Assert(theRec->find(strWorld, wi));
	return wi->sym() ?
		((const UniqIdSym&)wi->sym()->cast(UniqIdSym::TheType)).val() : UniqId();
}

RndDistr *AgentSym::httpVersions(const TokenIdentifier &versionNames) const {
	return namesToDistr(strHttp_versions, versionNames);
}

SocketSym *AgentSym::socket() const {
	SynSymTblItem *si = 0;
	Assert(theRec->find(strSocket, si));
	Assert(si->sym());
	return &(SocketSym&)si->sym()->cast(SocketSym::TheType);
}

RndDistr *AgentSym::xactLifetime() const {
	return getDistr(strXact_lifetime);
}

RndDistr *AgentSym::xactThink() const {
	return getDistr(strXact_think);
}

RndDistr *AgentSym::pconnUseLmt() const {
	return getDistr(strPconn_use_lmt);
}

Time AgentSym::idlePconnTimeout() const {
	return getTime(strIdle_pconn_tout);
}

PopModelSym *AgentSym::popModel() const {
	const SynSym *pms = getRecSym(strPop_model);
	return pms ?
		&((PopModelSym&)pms->cast(PopModelSym::TheType)) : 0;
}

bool AgentSym::msgTypes(Array<StringSym*> &types, Array<double> &tprobs) const {
	if (ArraySym *a = getArraySym(msgTypesField())) {
		a->copyProbs(tprobs);
		a->exportA(types);
		return true;
	}
	return false;
}

RndDistr *AgentSym::msgTypes(const TokenIdentifier &typeNames) const {
	return namesToDistr(msgTypesField(), typeNames);
}

bool AgentSym::abortProb(double &prob) const {
        return getDouble(strAbort_prob, prob);
}

bool AgentSym::cookieSender(double &prob) const {
        return getDouble(strCookie_sender, prob);
}

bool AgentSym::customStatsScope(Array<StringSym*> &syms) const {
	if (const ArraySym *const a = getArraySym(strCustomStatsScope)) {
		a->exportA(syms);
		return true;
	}
	return false;
}

const ArraySym *AgentSym::httpHeaders() const {
	return getArraySym(strHttpHeaders);
}
