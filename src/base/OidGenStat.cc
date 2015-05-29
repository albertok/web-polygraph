
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ILog.h"
#include "base/OLog.h"
#include "base/OidGenStat.h"


OidGenStat::OidGenStat() {
	reset();
}

void OidGenStat::reset() {
	memset(theNeed, 0, sizeof(theNeed));
	memset(theLack, 0, sizeof(theLack));
	memset(theGen, 0, sizeof(theGen));
	waitForGen = false;
}

void OidGenStat::recordNeed(bool rep, int world) {
	Assert(!waitForGen);
	needWorld = world;
	needRep = rep;
	waitForGen = true;
}

void OidGenStat::recordGen(bool rep, int world) {
	Assert(waitForGen);
	theNeed[toIdx(needRep, needWorld)]++;
	if (needRep != rep || needWorld != world)
		theLack[toIdx(needRep, needWorld)]++;
	theGen[toIdx(rep, world)]++;
	waitForGen = false;
}

OidGenStat &OidGenStat::operator +=(const OidGenStat &s) {
	mergeTable(theNeed, s.theNeed);
	mergeTable(theLack, s.theLack);
	mergeTable(theGen, s.theGen);
	return *this;
}

void OidGenStat::store(OLog &log) const {
	writeTable(log, theNeed);
	writeTable(log, theLack);
	writeTable(log, theGen);
}

void OidGenStat::load(ILog &log) {
	readTable(log, theNeed);
	readTable(log, theLack);
	readTable(log, theGen);
}

ostream &OidGenStat::print(ostream &os, const String &pfx) const {
	os << pfx << "categories.count:\t " << 3 << endl;
	printTable(os, pfx + "need.", theNeed);
	printTable(os, pfx + "lack.", theLack);
	printTable(os, pfx + "gen.", theGen);
	return os;
}

void OidGenStat::mergeTable(Table &a, const Table &b) const {
	for (int i = 0; i < 8; ++i)
		a[i] += b[i];
}

void OidGenStat::writeTable(OLog &log, const Table &t) const {
	for (int i = 0; i < 8; ++i)
		log << t[i];
}

void OidGenStat::readTable(ILog &log, Table &t) {
	for (int i = 0; i < 8; ++i)
		log >> t[i];
}

void OidGenStat::printTable(ostream &os, const String &pfx, const Table &t) const {
	os << pfx << "new_prv.count:\t     " << t[toIdx(false, intPrivate)] << endl;
	os << pfx << "new_pub.count:\t     " << t[toIdx(false, intPublic)] << endl;
	os << pfx << "rep_prv.count:\t     " << t[toIdx(true, intPrivate)] << endl;
	os << pfx << "rep_pub.count:\t     " << t[toIdx(true, intPublic)] << endl;
	os << pfx << "new_foreign.count:\t " << t[toIdx(false, intForeign)] << endl;
	os << pfx << "rep_foreign.count:\t " << t[toIdx(true, intForeign)] << endl;
}
