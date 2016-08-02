
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/sstream.h"

#include "xstd/Assert.h"
#include "xstd/Rnd.h"
#include "xstd/gadgets.h"
#include "base/RndPermut.h"
#include "pgl/CredentialsGen.h"


StringArrayBlock::TypeAnchor CredentialsGen::TheTypeAnchor;
String CredentialsGen::ThePfx = "user";
char CredentialsGen::TheSfx = 'x';
int CredentialsGen::TheNextId = 1;

CredentialsGen::CredentialsGen(): StringArrayBlock(&TheTypeAnchor), 
	theId(TheNextId++), theCount(0) {
}

StringArrayBlock *CredentialsGen::clone() const {
	CredentialsGen *g = new CredentialsGen;
	g->theId = theId;
	g->configure(theCount, theNamespace);
	return g;
}

void CredentialsGen::configure(int aCount, const String &aNamespace) {
	theCount = aCount;
	theNamespace = aNamespace;
}

int CredentialsGen::count() const {
	return theCount;
}

String CredentialsGen::item(int idx) const {
	if (!Should(0 <= idx && idx < theCount))
		return String();

	RndGen rng(GlbPermut(idx, theId));
	ostringstream buf;
	// use 'idx' to ensure uniqueness within the namespace
	buf << ThePfx << idx << '_' << hex << (rng.trial32u() % 0xFFFF)
		<< '@' << theNamespace
		<< ":pw" << hex << rng.trial32u() << TheSfx << ends;

	const String res = buf.str().c_str();
	streamFreeze(buf, false);
	return res;
}

bool CredentialsGen::find(const Area &member, int &idx) const {
	if (member.size() <= ThePfx.len() || ThePfx.cmp(member.data(), ThePfx.len()) != 0)
		return false;

	if (member.data()[member.size()-1] != TheSfx)
		return false;

	int i = -1;
	if (!isInt(member.data() + ThePfx.len(), i))
		return false;

	if (!(0 <= i && i < theCount))
		return false;

	const String m = item(i);
	if (m.len() != member.size() || m.cmp(member.data(), member.size()) != 0)
		return false;

	idx = i;
	return true;
}

bool CredentialsGen::canMergeSameType(const StringArrayBlock &) const {
	return false;
}

void CredentialsGen::mergeSameType(const StringArrayBlock &) {
	Assert(false); // should not be called, see canMergeSameType()
}
