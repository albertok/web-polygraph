
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/SslPhaseStat.h"

/* SslPhaseStat::Stat */

SslPhaseStat::Stat::Stat(): theNew(0), theReused(0) {
}

void SslPhaseStat::Stat::record(const SessionKind kind) {
	switch (kind) {
		case skNew:
			++theNew;
			break;
		case skReused:
			++theReused;
			break;
		default:
			Should(false);
	}
}

SslPhaseStat::Stat &SslPhaseStat::Stat::operator +=(const Stat &s) {
	theNew += s.theNew;
	theReused += s.theReused;
	return *this;
}

OLog &SslPhaseStat::Stat::store(OLog &log) const {
	return	log
		<< theNew
		<< theReused
		;
}

ILog &SslPhaseStat::Stat::load(ILog &log) {
	return log
		>> theNew
		>> theReused
		;
}

ostream &SslPhaseStat::Stat::print(ostream &os, const String &pfx) const {
	return os
		<< pfx << "new:\t " << theNew << endl
		<< pfx << "reused:\t " << theReused << endl
		;
}

/* SslPhaseStat */

SslPhaseStat &SslPhaseStat::operator +=(const SslPhaseStat &s) {
	theOffered += s.theOffered;
	theMeasured += s.theMeasured;
	return *this;
}

OLog &SslPhaseStat::store(OLog &log) const {
	return log
		<< theOffered
		<< theMeasured
		;
}

ILog &SslPhaseStat::load(ILog &log) {
	return log
		>> theOffered
		>> theMeasured
		;
}

ostream &SslPhaseStat::print(ostream &os, const String &pfx) const {
	theOffered.print(os, pfx + "offered.");
	theMeasured.print(os, pfx + "measured.");
	return os;
}
