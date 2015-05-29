
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "pgl/PglRec.h"
#include "pgl/PglRecSym.h"


PglRec::PglRec() {
}

PglRec::~PglRec() {
}

SynSymTblItem **PglRec::valp(const String &key) {
	// we are responsible for the first component of a dot-name only
	String ancor = key;
	String tail;
	if (const char *p = key.chr('.')) {
		ancor = key(0, p-key.cstr());
		tail = key(p+1 - key.cstr(), key.len());
	}

	// lookup ancor
	if (SynSymTblItem **ip = SynSymTbl::valp(ancor)) {
		SynSymTblItem *i = *ip;
		Assert(i);

		// check if we need to go further
		if (tail) {
			if (i->sym() && i->sym()->isA(RecSym::TheType)) {
				RecSym &s = (RecSym &) i->sym()->cast(RecSym::TheType);
				return s.rec()->valp(tail);
			}
			// would be nice to complain here, but we lack key's location
			return 0;
		}
		return ip;
	}
	return 0;
}

PglRec *PglRec::clone() const {
	PglRec *rec = new PglRec;

	for (int i = 0; i < count(); ++i) {
		if (valAt(i)) {
			SynSymTblItem *clone = new SynSymTblItem(*valAt(i));

			if (SynSym *s = valAt(i)->sym())
				clone->sym(s->clone(s->type()));

			rec->add(keyAt(i), clone);
		}
	}
	return rec;
}

// Note: parameter types are not checked
void PglRec::copyCommon(const PglRec &rec) {
	for (int i = 0; i < count(); ++i) {
		SynSymTblItem *val = 0;
		if (!rec.find(keyAt(i), val) || !val)
			continue;

		SynSymTblItem *const clone = new SynSymTblItem(*val);
		if (const SynSym *const s = val->sym())
			clone->sym(s->clone(s->type()));

		delete valAt(i);
		valAt(i) = clone;
	}
}

SynSymTblItem *PglRec::bAdd(const String &type, const String &name, SynSym *s) {
	SynSymTblItem *member = new SynSymTblItem(type, name);
	member->sym(s);
	add(member->name(), member);
	return member;
}
