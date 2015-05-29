
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/AuthStat.h"

AuthStat::AuthStat() {
	reset();
}

void AuthStat::reset() {
	theAuthIng.reset();
	theAuthEd.reset();
}

OLog &AuthStat::store(OLog &log) const {
	return log << theAuthIng << theAuthEd;
}

ILog &AuthStat::load(ILog &log) {
	return log >> theAuthIng >> theAuthEd;
}

bool AuthStat::sane() const {
	return theAuthIng.sane() && theAuthEd.sane();
}

void AuthStat::authIngAll(TmSzStat &all) const {
	theAuthIng.all(all);
}

void AuthStat::authEdAll(TmSzStat &all) const {
	theAuthEd.all(all);
}

AuthStat &AuthStat::operator +=(const AuthStat &s) {
	theAuthIng += s.theAuthIng;
	theAuthEd += s.theAuthEd;
	return *this;
}

ostream &AuthStat::print(ostream &os, const String &pfx) const {
	theAuthIng.print(os, pfx + "ing.");
	theAuthEd.print(os, pfx + "ed.");
	return os;
}
