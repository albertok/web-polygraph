
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"
#include "xparser/xparser.h"

#include "pgl/PglCtx.h"
#include "pgl/PglRec.h"
#include "pgl/PglRecSym.h"

PglCtx *PglCtx::TheRootCtx = new PglCtx(String());


PglCtx::PglCtx(const String &aName, PglCtx *aParent): 
	theName(aName), theParent(aParent) {
}

PglCtx::~PglCtx() {
}

SynSymTblItem **PglCtx::valp(const String &key) {
	// if named, try searching with our name as prefix
	if (theName) { 
		String fullName = theName;
		fullName += ".";
		fullName += key;
		if (SynSymTblItem **i = memberp(fullName))
			return i;
	}

	// now search without a prefix since prefix does not help

	// if it is a complex name, lookup rules change
	if (key.chr('.'))
		return memberp(key);

	// standard recursive lookup
	return valpR(key);
}

// recursive valp
SynSymTblItem **PglCtx::valpR(const String &key) {
	Assert(!key.chr('.'));

	// check local namespace
	if (SynSymTblItem **i = SynSymTbl::valp(key))
		return i;

	// ask parent
	return theParent ? theParent->valp(key) : 0;
}

// searches for a symbol that corresponds to a composite name
SynSymTblItem **PglCtx::memberp(const String &name) {
	const char *suffix = name.chr('.');
	Assert(suffix);
	const String head = name(0, suffix - name.cstr()); // first component

	// find anchor
	SynSymTblItem **ip = valpR(head);
	if (!ip)
		return 0;

	SynSymTblItem *i = *ip;
	if (!i || !i->sym())
		return 0;

	const String tail = name(suffix+1 - name.cstr(), name.len());
	return i->sym()->memberItem(tail); // may return null
}

int PglCtx::nestDepth() const {
	return 1 + (theParent ? theParent->nestDepth() : 0);
}

void PglCtx::noteAdd(int idx) {
	theItems[idx].theVal->ctx(this);
}

void PglCtx::noteDel(int idx) {
	theItems[idx].theVal->ctx(0);
}

void PglCtx::report(ostream &os, const String &pfx) const {
	if (theName)
		os << pfx << theName << ':' << endl;
	SynSymTbl::report(os, pfx);
}
