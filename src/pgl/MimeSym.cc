
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/MimeSym.h"



String MimeSym::TheType = "Mime";

static String strExtensions = "extensions";
static String strPrefixes = "prefixes";
static String strStringArr = "string[]";
static String strType = "type";
static String strMimeExtensions = "mime-extensions";
static String strMimePrefixes = "mime-prefixes";


MimeSym::MimeSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(StringSym::TheType, strType, 0);
	theRec->bAdd(strStringArr, strPrefixes, 0);   // XXX: move to UrlGen
	theRec->bAdd(strStringArr, strExtensions, 0); // XXX: move to UrlGen
}

MimeSym::MimeSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool MimeSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *MimeSym::dupe(const String &type) const {
	if (isA(type))
		return new MimeSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

String MimeSym::mimeType() const {
	return getString(strType);
}

bool MimeSym::extensions(Array<String*> &exts, RndDistr *&selector) const {
	if (ArraySym *as = getArraySym(strExtensions)) {
		selector = as->makeSelector(strMimeExtensions);
		Assert(getStrings(strExtensions, exts));
		return true;
	}
	return false;
}

bool MimeSym::prefixes(Array<String*> &pxs, RndDistr *&selector) const {
	if (ArraySym *as = getArraySym(strPrefixes)) {
		selector = as->makeSelector(strMimePrefixes);
		Assert(getStrings(strPrefixes, pxs));
		return true;
	}
	return false;
}
