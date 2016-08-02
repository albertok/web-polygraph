
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/BStream.h"
#include "csm/ContentMgr.h"
#include "runtime/ObjGalaxy.h"


const UniqId &ObjGalaxy::id() const {
	Assert(!theObjWorlds.empty());
	return theObjWorlds.front().id();
}

void ObjGalaxy::id(const UniqId &id) {
	Assert(theObjWorlds.empty());
	const int count = Capacity();
	theObjWorlds.reserve(count);
	for (int i = 0; i < count; ++i) {
		ObjWorld world;
		world.id(id);
		world.type(i);
		theObjWorlds.push_back(world);
	}
}

Counter ObjGalaxy::currentWss(bool &isFrozen) const {
	isFrozen = true;
	Counter wss = 0;
	for (ObjWorlds::size_type i = 0; i < theObjWorlds.size(); ++i) {
		const ObjWorld &world = theObjWorlds[i];
		if (world.wss() >= 0)
			wss += world.wss();
		else {
			isFrozen = false;
			wss += world.size();
		}
	}
	return wss;
}

Counter ObjGalaxy::size() const {
	Counter sum = 0;
	for (ObjWorlds::size_type i = 0; i < theObjWorlds.size(); ++i)
		sum += theObjWorlds[i].size();
	return sum;
}

void ObjGalaxy::freezeWss() {
    for (ObjWorlds::size_type i = 0; i < theObjWorlds.size(); ++i)
		theObjWorlds[i].freezeWss();
}

void ObjGalaxy::store(OBStream &os) const {
	os << ContainerStreamer(theObjWorlds);
}

void ObjGalaxy::load(IBStream &is) {
    is >> (ContainerStreamer(theObjWorlds));
}

void ObjGalaxy::dumpWorlds(ostream &os) const {
	for (ObjWorlds::size_type i = 0; i < theObjWorlds.size(); ++i)
		os << "\t\ttype " << i << ": " << theObjWorlds[i] << endl;
}

void ObjGalaxy::update(const ObjWorld &newWorld) {
	ObjWorld &w = world(newWorld.type());
	Assert(w.id() == newWorld.id());
	// try to preserve consistency: update all or nothing
	if (newWorld.newer(w)) {
		w.size(newWorld.size());
		w.wss(newWorld.wss());
		w.hotPos(newWorld.hotPos());
	}
}

const ObjWorld &ObjGalaxy::world(const int type) const {
	return const_cast<ObjGalaxy *>(this)->world(type);
}

ObjWorld &ObjGalaxy::world(const int type) {
	Assert(0 <= type && type < static_cast<int>(theObjWorlds.size()));
	return theObjWorlds[type];
}

int ObjGalaxy::Capacity() {
	return TheContentMgr.count();
}
