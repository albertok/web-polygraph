
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/MimeSym.h"
#include "pgl/ObjLifeCycleSym.h"
#include "pgl/ContentSym.h"
#include "pgl/MimeHeaderSym.h"
#include "pgl/ClientBehaviorSym.h"


String ContentSym::TheType = "Content";

static String strContentArr = "Content[]";
static String strCachable = "cachable";
static String strChecksum = "checksum";
static String strChoice_space = "choice_space";
static String strClient_behavior = "client_behavior";
static String strGenerator = "generator";
static String strContent_db = "content_db";
static String strEmbedded_obj_cnt = "embedded_obj_cnt";
static String strInfect_prob = "infect_prob";
static String strInject_object = "inject_object";
static String strInject_db = "inject_db";
static String strInject_gap = "inject_gap";
static String strInt_distr = "int_distr";
static String strKind = "kind";
static String strMay_contain = "may_contain";
static String strMime = "mime";
static String strObj_life_cycle = "obj_life_cycle";
static String strUnique = "unique";
static String strSize = "size";
static String strSize_distr = "size_distr";
static String strContainerContents = "container_contents";
static String strStringArr = "string[]";
static String strEncodings = "encodings";
static String strMimeHeaders = "mime_headers";
static String strMimeHeaderArr = "MimeHeader[]";
static String strDocumentRoot = "document_root";

ContentSym::ContentSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(StringSym::TheType, strKind, 0);
	theRec->bAdd(MimeSym::TheType, strMime, new MimeSym);
	theRec->bAdd(strSize_distr, strSize, 0);
	theRec->bAdd(ObjLifeCycleSym::TheType, strObj_life_cycle, new ObjLifeCycleSym);
	theRec->bAdd(NumSym::TheType, strCachable, 0);
	theRec->bAdd(NumSym::TheType, strChecksum, 0);
	theRec->bAdd(NumSym::TheType, strUnique, 0);
	theRec->bAdd(strContentArr, strMay_contain, 0);
	theRec->bAdd(strInt_distr, strEmbedded_obj_cnt, 0);
	theRec->bAdd(IntSym::TheType, strChoice_space, 0);
	theRec->bAdd(ClientBehaviorSym::TheType, strClient_behavior, new ClientBehaviorSym);
	theRec->bAdd(StringSym::TheType, strGenerator, 0);
	theRec->bAdd(StringSym::TheType, strContent_db, 0);
	theRec->bAdd(StringSym::TheType, strInject_object, 0);
	theRec->bAdd(StringSym::TheType, strInject_db, 0);
	theRec->bAdd(NumSym::TheType, strInfect_prob, 0);
	theRec->bAdd(strSize_distr, strInject_gap, 0);
	theRec->bAdd(strStringArr, strEncodings, 0);
	theRec->bAdd(strMimeHeaderArr, strMimeHeaders, 0);
	theRec->bAdd(StringSym::TheType, strDocumentRoot, 0);
}

ContentSym::ContentSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool ContentSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *ContentSym::dupe(const String &type) const {
	if (isA(type))
		return new ContentSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

bool ContentSym::hasEmbed() const {
	SynSymTblItem *conti = 0;
	Assert(theRec->find(strMay_contain, conti));
	return conti->sym() != 0;
}

String ContentSym::kind() const {
	return getString(strKind);
}

MimeSym *ContentSym::mime() const {
	SynSymTblItem *mi = 0;
	Assert(theRec->find(strMime, mi));
	Assert(mi->sym());
	return &(MimeSym&)mi->sym()->cast(MimeSym::TheType);
}

RndDistr *ContentSym::size() const {
	return getDistr(strSize);
}

ObjLifeCycleSym *ContentSym::objLifeCycle() const {
	SynSymTblItem *olci = 0;
	Assert(theRec->find(strObj_life_cycle, olci));
	Assert(olci->sym());
	return &(ObjLifeCycleSym&)olci->sym()->cast(ObjLifeCycleSym::TheType);
}

bool ContentSym::cachable(double &ratio) const {
	return getDouble(strCachable, ratio);
}

bool ContentSym::checksum(double &ratio) const {
	return getDouble(strChecksum, ratio);
}

bool ContentSym::unique(double &ratio) const {
	return getDouble(strUnique, ratio);
}

bool ContentSym::contains(Array<ContentSym*> &ccfgs, RndDistr *&selector, Array<double> &cprobs) const {
	SynSymTblItem *conti = 0;
	Assert(theRec->find(strMay_contain, conti));
	if (!conti->sym())
		return false; // undefined

	ArraySym &a = (ArraySym&)conti->sym()->cast(ArraySym::TheType);
	selector = a.makeSelector(strContainerContents);
	ccfgs.stretch(a.count());
	cprobs.stretch(a.count());
	a.copyProbs(cprobs);
	for (int i = 0; i < a.count(); ++i) {
		ContentSym *csym = &((ContentSym&)a[i]->cast(TheType));
		ccfgs.append(csym);
	}

	return true;
}

RndDistr *ContentSym::embedCount() const {
	return getDistr(strEmbedded_obj_cnt);
}

bool ContentSym::choiceSpace(int &space) const {
	return getInt(strChoice_space, space);
}

ClientBehaviorSym *ContentSym::clientBehavior() const {
	SynSymTblItem *mi = 0;
	Assert(theRec->find(strClient_behavior, mi));
	Assert(mi->sym());
	return &(ClientBehaviorSym&)mi->sym()->cast(ClientBehaviorSym::TheType);
}

String ContentSym::generator() const {
	return getString(strGenerator);
}

String ContentSym::cdb() const {
	return getString(strContent_db);
}

String ContentSym::injectObject() const {
	return getString(strInject_object);
}

String ContentSym::injectDb() const {
	return getString(strInject_db);
}

bool ContentSym::infectProb(double &prob) const {
	return getDouble(strInfect_prob, prob);
}

RndDistr *ContentSym::injectGap() const {
	return getDistr(strInject_gap);
}

bool ContentSym::encodings(Array<String*> &codings) const {
	return getStrings(strEncodings, codings);
}

const ArraySym *ContentSym::mimeHeaders() const {
	return getArraySym(strMimeHeaders);
}

String ContentSym::documentRoot() const {
	return getString(strDocumentRoot);
}
