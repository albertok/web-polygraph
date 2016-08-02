
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
	theType = -1;
	theHotSet.reset();
	theSize = 0;
	theWss = -1;
}

bool ObjWorld::newer(const ObjWorld &w) const {
	Assert(*this);
	Assert(id() == w.id());
	Assert(theType == w.theType);
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
	Assert(*this);
	Assert(oid.type() == theType);
	Assert(sel);

	oid.repeat(true);
	oid.world(theId);
	sel->choose(theSize, theWss, theHotSet.pos(), oid);
	calcPrefix(oid);
}

void ObjWorld::produce(ObjId &oid, RndGen &) {
	Assert(*this);
	Assert(oid.type() == theType);

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
	os << theId << theType << theSize << theWss;
	os.puti64(theHotSet.pos());
}

void ObjWorld::load(IBStream &is) {
	is >> theId >> theType >> theSize >> theWss;
	theHotSet.pos(is.geti64());
}

ostream &ObjWorld::print(ostream &os) const {
	return os << theId 
		<< " t" << theType
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
		const Counter numberOfNew = oid.name() * newProb + 1;
		// if theWss is zero, we are being forced to produce
		const Counter prefix = (theWss <= 0 || numberOfNew <= theWss) ?
			oid.name() % numberOfNew + 1 :
			numberOfNew - oid.name() % theWss;
		oid.prefix(prefix);
	}
}

bool ObjWorld::parse(const char *buf, const char *end) {
	UniqId id;
	if (!id.parse(buf, end) || *buf != ' ')
		return false;

	int type = -1;
	int size = -1, wss = -1;
	int hotPos = -1;
	const char *p = buf + 1;

	// Id tType Wss/Size HotPos
	if (*p == 't' && isInt(p+1, theType, &p) &&
		theType >= 0 && *p == ' ' &&
		isInt(p+1, wss, &p) && *p == '/' &&
		isInt(p+1, size, &p) && *p == ' ' &&
		isInt(p+1, hotPos)) {
		theId = id;
		theType = type;
		theSize = size;
		theWss = wss;
		theHotSet.pos(hotPos);
		return true;
	}

	return false;
}
