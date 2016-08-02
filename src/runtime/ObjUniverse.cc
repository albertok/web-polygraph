
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Rnd.h"
#include "base/BStream.h"
#include "runtime/HostMap.h"
#include "runtime/LogComment.h"
#include "runtime/ObjUniverse.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/Viservs.h"


int ObjUniverse::TheCount = 0;


ObjUniverse *ObjUniverse::Add(HostCfg *host, ObjUniverse *universe) {
	if (!Should(host && universe))
		return 0;

	//cerr << here << "ObjUniverse::Add(" << host->theAddr << ") = " << universe << endl;

	if (Should(host->theUniverse == 0)) {
		host->theUniverse = universe;
		TheCount++;
	} else
	if (host->theUniverse != universe) {
		delete universe;
	}
	return host->theUniverse;
}

ObjUniverse *ObjUniverse::Put(HostCfg *host, ObjUniverse *universe) {
	if (!Should(host))
		return 0;

	//cerr << here << "ObjUniverse::Put(" << host->theAddr << ") = " << universe << endl;

	if (host->theUniverse && host->theUniverse != universe) {
		delete host->theUniverse;
		host->theUniverse = 0;
		TheCount--;
	}
	return Add(host, universe);
}

int ObjUniverse::Count() {
	return TheCount;
}

bool ObjUniverse::Frozen() {
	int frozenCount, totalCount;
	CurrentWss(frozenCount, totalCount);
	return frozenCount == totalCount;
}

void ObjUniverse::FreezeWss() {
	for (ViservIterator i; !i.atEnd(); ++i)
		i.universe()->freezeWss();
}

Counter ObjUniverse::CurrentWss(int &frozenCount, int &totalCount) {
	totalCount = frozenCount = 0;
	Counter wss = 0;
	for (ViservIterator i; !i.atEnd(); ++i) {
		int frozen = 0, total = 0;
		wss += i.universe()->currentWss(frozen, total);
		frozenCount += frozen;
		totalCount += total;
	}
	return wss;
}

void ObjUniverse::DumpGalaxies(ostream &os) {
	for (ViservIterator i; !i.atEnd(); ++i) {
		os << i.universe() << "->world-" << i.viserv() << ":" << endl;
		i.universe()->dumpGalaxies(os);
	}
}

ObjUniverse::ObjUniverse(): theLocalGalaxy(-1),
	theNextWorldToSync(0), theNextLocalWorldToSync(0) {
}

ObjUniverse::ObjUniverse(const UniqId &localId): theLocalGalaxy(0),
	theNextWorldToSync(0), theNextLocalWorldToSync(0) {
	//cerr << here << "new local ObjUniverse id: " << localId << endl;
	Assert(localId);
	ObjGalaxy local;
	local.id(localId);
	theGalaxies.push_back(local);
}

bool ObjUniverse::find(const UniqId &id, int &galaxyIdx) const {
	// XXX: slow
	for (Galaxies::size_type i = 0; i < theGalaxies.size(); ++i) {
		if (theGalaxies[i].id() == id) {
			galaxyIdx = i;
			return true;
		}
	}
	return false;
}

const ObjWorld *ObjUniverse::worldToSync() const {
	if (theGalaxies.empty())
		return 0;

	return &world(theNextWorldToSync++);
}

const ObjWorld *ObjUniverse::localWorldToSync() const {
	if (theGalaxies.empty())
		return 0;

	const int idx = theNextLocalWorldToSync++ % ObjGalaxy::Capacity();
	return &localGalaxy().world(idx);
}

const ObjWorld *ObjUniverse::newerWorld(const ObjWorld &oldWorld, int idx) const {
	idx = idx * ObjGalaxy::Capacity() + oldWorld.type();
	for (int count = theGalaxies.size() * ObjGalaxy::Capacity();
		count--; ++idx) {
		const ObjWorld &w = world(idx);
		if (w.id()) {
			if (w.id() == oldWorld.id()) {
				if (w.newer(oldWorld)) // known, newer world
					return &w;
			} else
				return &w; // unknown (other) world, possibly new
		}
	}
	// there is only one galaxy and it is not newer
	return 0;
}

ObjGalaxy &ObjUniverse::getGalaxy(const WorldKind kind, const int type) {
	int count = theGalaxies.size();
	static RndGen rng;
	for (int idx = rng(0, count); count--; ++idx) {
		idx %= theGalaxies.size();
		ObjGalaxy &galaxy = theGalaxies[idx];
		if ((galaxy.world(type).*kind)())
			return galaxy;
	}

	Assert(false); // getGalaxy must be called if can"Kind"() only
	return localGalaxy();
}

// fails if there is no local galaxy (e.g., server side)
ObjGalaxy &ObjUniverse::localGalaxy() {
	return galaxyAt(theLocalGalaxy);
}

const ObjGalaxy &ObjUniverse::localGalaxy() const {
	return galaxyAt(theLocalGalaxy);
}

const ObjGalaxy &ObjUniverse::galaxyAt(const Galaxies::size_type idx) const {
	Assert((!idx || 0 < idx) && idx < theGalaxies.size());
	return theGalaxies[idx];
}

ObjGalaxy &ObjUniverse::galaxyAt(const Galaxies::size_type idx) {
	Assert((!idx || 0 < idx) && idx < theGalaxies.size());
	return theGalaxies[idx];
}

Counter ObjUniverse::currentWss(int &frozenCount, int &totalCount) const {
	totalCount = frozenCount = 0;
	Counter wss = 0;
	for (Galaxies::size_type i = 0; i < theGalaxies.size(); ++i) {
		++totalCount;
		bool frozen;
		wss += theGalaxies[i].currentWss(frozen);
		if (frozen)
			++frozenCount;
	}
	return wss;
}

Counter ObjUniverse::size() const {
	Counter sum = 0;
	for (Galaxies::size_type i = 0; i < theGalaxies.size(); ++i)
		sum += theGalaxies[i].size();
	return sum;
}

const ObjWorld &ObjUniverse::world(const unsigned idx) const {
	const int galaxy = (idx / ObjGalaxy::Capacity()) % theGalaxies.size();
	const int type = idx % ObjGalaxy::Capacity();
	return theGalaxies[galaxy].world(type);
}

// XXX: these kind of loops are slow
bool ObjUniverse::canRepeat(const int type) const {
	for (Galaxies::size_type i = 0; i < theGalaxies.size(); ++i) {
		if (theGalaxies[i].world(type).canRepeat())
			return true;
	}
	return false;
}

bool ObjUniverse::canRepeatLocal(const int type) const {
	return localGalaxy().world(type).canRepeat();
}

bool ObjUniverse::canProduce(const int type) const {
	// to avoid race conditions (two robots producing a false miss), 
	// only local galaxies can produce
	return theLocalGalaxy >= 0 && localGalaxy().world(type).canProduce();
}

void ObjUniverse::repeat(ObjId &oid, ObjSelector *sel) {
	ObjGalaxy &galaxy = getGalaxy(&ObjWorld::canRepeat, oid.type());
	galaxy.world(oid.type()).repeat(oid, sel);
}

void ObjUniverse::repeatLocal(ObjId &oid, ObjSelector *sel) {
	Assert(theLocalGalaxy >= 0);
	localGalaxy().world(oid.type()).repeat(oid, sel);
}

void ObjUniverse::produce(ObjId &oid, RndGen &rng) {
	Assert(theLocalGalaxy >= 0);
	localGalaxy().world(oid.type()).produce(oid, rng);
}

void ObjUniverse::freezeWss() {
	localGalaxy().freezeWss();
}

void ObjUniverse::update(const ObjWorld &world) {
	int galaxyIdx;
	if (find(world.id(), galaxyIdx))
		theGalaxies[galaxyIdx].update(world);
	else
		add(world);
}

void ObjUniverse::add(const ObjWorld &world) {
	ObjGalaxy galaxy;
	galaxy.id(world.id());
	galaxy.update(world);
	theGalaxies.push_back(galaxy);
	//cerr << here << this << "->universe adding galaxy " << galaxy.id() << endl;
}

void ObjUniverse::store(OBStream &os) const {
	os << ContainerStreamer(theGalaxies) << theLocalGalaxy
		<< theNextWorldToSync << theNextLocalWorldToSync;
}

void ObjUniverse::load(IBStream &is) {
	is >> ContainerStreamer(theGalaxies) >> theLocalGalaxy
		>> theNextWorldToSync >> theNextLocalWorldToSync;
	if (!Should(-1 <= theLocalGalaxy && theLocalGalaxy < static_cast<int>(theGalaxies.size())))
		theLocalGalaxy = -1;
}

void ObjUniverse::dumpGalaxies(ostream &os) const {
	for (Galaxies::size_type i = 0; i < theGalaxies.size(); ++i) {
		os << "\t" << &theGalaxies[i] << "->galaxy-" << i <<
			(theLocalGalaxy == static_cast<int>(i) ? " *" : "")
			<< endl;
		theGalaxies[i].dumpWorlds(os);
		if (theLocalGalaxy == static_cast<int>(i))
			os << " *";
	}
}

void ObjUniverse::ReportWss(int commentLvl) {
	const BigSize meanFillSz = 
		(*TheStatPhaseMgr && TheStatPhaseMgr->fillCnt()) ?
		TheStatPhaseMgr->fillSz()/TheStatPhaseMgr->fillCnt() :
		BigSize(0);

	ostream &os = Comment(commentLvl) << 
		"fyi: min 'direct' objects in working set:" << endl;

	int globalFrozenCount = 0;
	int globalTotalCount = 0;
	const Counter globalWss = CurrentWss(globalFrozenCount, globalTotalCount);
	os << "\tglobal public: " << globalWss << " (";
	if (meanFillSz > 0)
		os << "~" << (meanFillSz*globalWss) << " size, ";
	os << globalFrozenCount << '/' << globalTotalCount << '=' <<
		Percent(globalFrozenCount, globalTotalCount) <<
		"% frozen)" << endl;

	os << endc;
}
