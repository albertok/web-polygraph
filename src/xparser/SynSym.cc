
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xparser/xparser.h"

#include "xstd/h/string.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xparser/SynSym.h"


/* SynSym */

SynSym::SynSym(const String &aType): theType(0) {
	type(aType);
}

SynSym::~SynSym() {
	type(0);
}

bool SynSym::isA(const String &type) const {
	return theType && type && type == theType;
}

// may be expensive
bool SynSym::canBe(const String &type) const {
	if (isA(type))
		return true;
	if (SynSym *s = clone(type)) {
		delete s;
		return true;
	}
	return false;
}

// expensive: comparison is done using print method!
bool SynSym::equal(const SynSym &s) const {
	ostringstream os1, os2;
	print(os1) << ends;
	s.print(os2) << ends;
	
	const bool res = os1.str() == os2.str();

	streamFreeze(os1, false);
	streamFreeze(os2, false);

	return res;
}

SynSym &SynSym::cast(const String &typ) {
	((const SynSym *)this)->cast(typ); // will abort if needed
	return *this;
}

const SynSym &SynSym::cast(const String &typ) const {
	if (!isA(typ)) {
		cerr << here << "cannot cast " << type() << " to " << typ <<
			endl << xabort;
	}
	return *this;
}

SynSym *SynSym::clone(const String &type) const {
	SynSym *copy = dupe(type);
	if (copy)
		copy->loc(loc());
	return copy;
}

void SynSym::type(const String &aType) {
	theType = aType;
}

SynSymTblItem **SynSym::memberItem(const String &) {
	return 0;
}

ostream &SynSym::print(ostream &os, const String &) const {
	return os << "<something " << type() << "-ish>";
}
