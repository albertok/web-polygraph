
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/AuthPhaseStat.h"

// TODO: add a check that this array is in sync with AuthPhaseStat::Scheme
const String AuthPhaseStat::SchemeNames[] = {
	"none",
	"basic",
	"ntlm",
	"negotiate",
	"ftp",
	"socks.userpass",
	"kerberos"
};

AuthPhaseStat::AuthPhaseStat() {
	Assert(sLast == sizeof(SchemeNames)/sizeof(*SchemeNames));
}

AuthPhaseStat::AuthPhaseStat(const AuthPhaseStat &s) {
	for (int i = 0; i < sLast; ++i)
		theStats[i] = s.theStats[i];
}

void AuthPhaseStat::reset() {
	for (int i = 0; i < sLast; ++i)
		theStats[i].reset();
}

OLog &AuthPhaseStat::store(OLog &log) const {
	for (int i = 0; i < sLast; ++i)
		if (theStats[i].active()) {
			log.putb(true);
			log << theStats[i];
		} else
			log.putb(false);
	return log;
}

ILog &AuthPhaseStat::load(ILog &log) {
	for (int i = 0; i < sLast; ++i)
		if (log.getb())
			log >> theStats[i];
		else
			theStats[i].reset();
	return log;
}

bool AuthPhaseStat::sane() const {
	for (int i = 0; i < sLast; ++i)
		if (!theStats[i].sane())
			return false;
	return true;
}

void AuthPhaseStat::all(TmSzStat &all) const {
	for (int i = 0; i < sLast; ++i)
		all += theStats[i];
}

AuthPhaseStat &AuthPhaseStat::operator +=(const AuthPhaseStat &s) {
	for (int i = 0; i < sLast; ++i)
		theStats[i] += s.theStats[i];
	return *this;
}

ostream &AuthPhaseStat::print(ostream &os, const String &pfx) const {
	for (int i = 0; i < sLast; ++i)
		theStats[i].print(os, pfx + SchemeNames[i] + '.');
	return os;
}
