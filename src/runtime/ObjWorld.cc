
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "base/BStream.h"
#include "base/ObjId.h"
#include "runtime/AddrMap.h"
#include "runtime/AddrMapItems.h"
#include "runtime/HostMap.h"
#include "runtime/ObjSelector.h"
#include "runtime/ObjWorld.h"


ObjWorld::ObjWorld() {
	reset();
}

void ObjWorld::reset() {
	theId.clear();
	theHotSet.reset();
	theSize = 0;
	theWss = -1;
}

bool ObjWorld::newer(const ObjWorld &w) const {
	Assert(id() == w.id());
	// we lack info to tell whether hotPos() is newer
	return size() > w.size() || wss() > w.wss();
}

bool ObjWorld::canRepeat() const {
	return theSize > 0 && theWss != 0;
}

bool ObjWorld::canProduce() const {
	return theWss != 0;
}

void ObjWorld::repeat(ObjId &oid, ObjSelector *sel) {
	// since selecting oid name affects oid type, type must not be pre-set
	Assert(oid.type() < 0);
	Assert(sel);

	oid.repeat(true);
	oid.world(theId);
	sel->choose(theSize, theWss, theHotSet.pos(), oid);
	calcPrefix(oid);
}

void ObjWorld::produce(ObjId &oid, RndGen &) {
	oid.repeat(false);
	oid.hot(false);
	oid.world(theId);
	oid.name(theSize+1);
	produced();
	calcPrefix(oid);
}

void ObjWorld::freezeWss() {
	theWss = theSize;
	theHotSet.syncPos(theSize, theWss);
}

void ObjWorld::produced() {
	++theSize;
	theHotSet.syncPos(theSize, theWss);
}

void ObjWorld::incWss(int &count) {
	if (theWss < 0)
		count++;
}

void ObjWorld::store(OBStream &os) const {
	os << theId << theSize << theWss;
	os.puti(theHotSet.pos());
}

void ObjWorld::load(IBStream &is) {
	is >> theId >> theSize >> theWss;
	theHotSet.pos(is.geti());
}

ostream &ObjWorld::print(ostream &os) const {
	return os << theId 
		<< ' ' << theWss << '/' << theSize 
		<< ' ' << theHotSet.pos();
}

void ObjWorld::calcPrefix(ObjId &oid) {
	const HostCfg *const host = TheHostMap->at(oid.viserv());
	if (host && host->theAddr.isDynamicName()) {
		Assert(TheAddrMap->has(host->theAddr));
		int niamIdx; // name in AddrMap index
		TheAddrMap->find(host->theAddr, niamIdx);
		const double newProb = TheAddrMap->itemAt(niamIdx).newProb();
		const int numberOfNew = (int)(oid.name() * newProb) + 1;
		const int prefix = (theWss < 0 || numberOfNew <= theWss) ?
			oid.name() % numberOfNew + 1 :
			numberOfNew - oid.name() % theWss;
		oid.prefix(prefix);
	}
}

bool ObjWorld::parse(const char *buf, const char *end) {
	UniqId id;
	if (!id.parse(buf, end) || *buf != ' ')
		return false;

	int size = -1, wss = -1;
	int hotPos = -1;
	const char *p = buf;

	if (isInt(p+1, wss, &p) && *p == '/' &&
		isInt(p+1, size, &p) && *p == ' ' &&
		isInt(p+1, hotPos)) {
		theId = id;
		theSize = size;
		theWss = wss;
		theHotSet.pos(hotPos);
		return true;
	}

	return false;
}
