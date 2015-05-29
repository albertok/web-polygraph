
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Rnd.h"
#include "base/ObjId.h"
#include "base/BStream.h"
#include "runtime/HostMap.h"
#include "runtime/Viservs.h"
#include "runtime/PubWorld.h"
#include "runtime/LogComment.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"


int PubWorld::TheCount = 0;


/* PubWorldSlice */

void PubWorldSlice::update(const ObjWorld &w) {
	// try to preserve consistency: update all or nothing
	if (w.newer(*this)) {
		size(w.size());
		wss(w.wss());
		hotPos(w.hotPos());
	}
}

/* PubWorld */

PubWorld *PubWorld::Add(HostCfg *host, PubWorld *pubWorld) {
	if (!Should(host && pubWorld))
		return 0;

	//cerr << here << "PubWorld::Add(" << host->theAddr << ") = " << pubWorld << endl;

	if (Should(host->thePubWorld == 0)) {
		host->thePubWorld = pubWorld;
		TheCount++;
	} else
	if (host->thePubWorld != pubWorld) {
		delete pubWorld;
	}
	return host->thePubWorld;		
}

PubWorld *PubWorld::Put(HostCfg *host, PubWorld *pubWorld) {
	if (!Should(host))
		return 0;

	//cerr << here << "PubWorld::Put(" << host->theAddr << ") = " << pubWorld << endl;

	if (host->thePubWorld && host->thePubWorld != pubWorld) {
		delete host->thePubWorld;
		host->thePubWorld = 0;
		TheCount--;
	}
	return Add(host, pubWorld);
}

int PubWorld::Count() {
	return TheCount;
}

bool PubWorld::Frozen() {
	int frozenCount, totalCount;
	CurrentWss(frozenCount, totalCount);
	return frozenCount == totalCount;
}

void PubWorld::FreezeWss() {
	for (ViservIterator i; !i.atEnd(); ++i)
		i.pubWorld()->freezeWss();
}

int PubWorld::CurrentWss(int &frozenCount, int &totalCount) {
	totalCount = frozenCount = 0;
	int wss = 0;
	for (ViservIterator i; !i.atEnd(); ++i) {
		int frozen = 0, total = 0;
		wss += i.pubWorld()->currentWss(frozen, total);
		frozenCount += frozen;
		totalCount += total;
	}
	return wss;
}

void PubWorld::DumpSlices(ostream &os) {
	for (ViservIterator i; !i.atEnd(); ++i) {
		os << i.pubWorld() << "->world-" << i.viserv() << ":" << endl;
		i.pubWorld()->dumpSlices(os);
	}
}

PubWorld::PubWorld(): theLocalSlice(-1), theNextSliceToSync(0) {
}

PubWorld::PubWorld(const UniqId &localId): theLocalSlice(0), theNextSliceToSync(0) {
	//cerr << here << "new local PubWorld id: " << localId << endl;
	Assert(localId);
	PubWorldSlice local;
	local.id(localId);
	theSlices.append(local);
}

bool PubWorld::find(const UniqId &id, int &sliceIdx) const {
	// XXX: slow
	for (sliceIdx = 0; sliceIdx < theSlices.count(); ++sliceIdx) {
		if (theSlices[sliceIdx].id() == id)
			return true;
	}
	return false;
}

const PubWorldSlice *PubWorld::sliceToSync() const {
	if (!theSlices.count())
		return 0;

	theNextSliceToSync %= theSlices.count();
	return &theSlices[theNextSliceToSync++];
}

const PubWorldSlice *PubWorld::newerSlice(const ObjWorld &oldSlice, int idx) const {
	for (int count = theSlices.count(); count--; ++idx) {
		idx %= theSlices.count();
		const PubWorldSlice &slice = theSlices[idx];
		if (slice) {
			if (slice.id() == oldSlice.id()) {
				if (slice.newer(oldSlice)) // known, newer slice
					return &slice;
			} else
				return &slice;  // unknown (other) slice, possibly new
		}
	}
	// there is only one slice and it is not newer
	return 0;
}

PubWorldSlice &PubWorld::getSlice(SliceKind kind) {
	int count = theSlices.count();
	static RndGen rng;
	for (int idx = rng(0, count); count--; ++idx) {
		idx %= theSlices.count();
		PubWorldSlice &slice = theSlices[idx];
		if ((slice.*kind)())
			return slice;
	}

	Assert(false); // getSlice must be called if can"Kind"() only
	return localSlice();
}

// fails if there is no local slice (e.g., server side)
PubWorldSlice &PubWorld::localSlice() {
	return sliceAt(theLocalSlice);
}

const PubWorldSlice &PubWorld::localSlice() const {
	return sliceAt(theLocalSlice);
}

const PubWorldSlice &PubWorld::sliceAt(int idx) const {
	Assert(0 <= idx && idx < theSlices.count());
	return theSlices[idx];
}

PubWorldSlice &PubWorld::sliceAt(int idx) {
	Assert(0 <= idx && idx < theSlices.count());
	return theSlices[idx];
}

int PubWorld::currentWss(int &frozenCount, int &totalCount) const {
	totalCount = frozenCount = 0;
	int wss = 0;
	for (int i = 0; i < theSlices.count(); ++i) {
		++totalCount;
		const int sliceWss = theSlices[i].wss();
		if (sliceWss >= 0) {
			++frozenCount;
			wss += sliceWss;
		} else {
			wss += theSlices[i].size();
		}
	}
	return wss;
}

int PubWorld::size() const {
	int sum = 0;
	for (int i = 0; i < theSlices.count(); ++i)
		sum += theSlices[i].size();
	return sum;
}

// XXX: these kind of loops are slow
bool PubWorld::canRepeat() const {
	for (int i = 0; i < theSlices.count(); ++i) {
		if (theSlices[i].canRepeat())
			return true;
	}
	return false;
}

bool PubWorld::canRepeatLocal() const {
	return localSlice().canRepeat();
}

bool PubWorld::canProduce() const {
	// to avoid race conditions (two robots producing a false miss), 
	// only local slices can produce 
	return theLocalSlice >= 0 && localSlice().canProduce();
}

void PubWorld::repeat(ObjId &oid, ObjSelector *sel) {
	PubWorldSlice &slice = getSlice(&PubWorldSlice::canRepeat);
	slice.repeat(oid, sel);
}

void PubWorld::repeatLocal(ObjId &oid, ObjSelector *sel) {
	Assert(theLocalSlice >= 0);
	localSlice().repeat(oid, sel);
}

void PubWorld::produce(ObjId &oid, RndGen &rng) {
	Assert(theLocalSlice >= 0);
	localSlice().produce(oid, rng);
}

void PubWorld::freezeWss() {
	localSlice().freezeWss();
}

void PubWorld::add(const PubWorldSlice &slice) {
	theSlices.append(slice);
	//cerr << here << this << "->PUBWORLD adding slice " << slice.id() << endl;
}

void PubWorld::store(OBStream &os) const {
	os << theSlices << theLocalSlice << theNextSliceToSync;
}

void PubWorld::load(IBStream &is) {
	is >> theSlices >> theLocalSlice >> theNextSliceToSync;
	if (!Should(-1 <= theLocalSlice && theLocalSlice < theSlices.count()))
		theLocalSlice = -1;
}

void PubWorld::dumpSlices(ostream &os) const {
	for (int i = 0; i < theSlices.count(); ++i) {
		os << "\t" << &theSlices[i] << "->slice-" << i << ":" <<
			theSlices[i];
		if (theLocalSlice == i)
				os << " *";
		os << endl;
	}
}

void PubWorld::ReportWss(int commentLvl) {
	const BigSize meanFillSz = 
		(*TheStatPhaseMgr && TheStatPhaseMgr->fillCnt()) ?
		TheStatPhaseMgr->fillSz()/TheStatPhaseMgr->fillCnt() :
		BigSize(0);

	ostream &os = Comment(commentLvl) << 
		"fyi: min 'direct' objects in working set:" << endl;

	int pubFrozenCount = 0;
	int pubTotalCount = 0;
	const int pubWss = CurrentWss(pubFrozenCount, pubTotalCount);
	os << "\tglobal public: " << pubWss << " (";
	if (meanFillSz > 0)
		os << "~" << (meanFillSz*pubWss) << " size, ";
	os << pubFrozenCount << '/' << pubTotalCount << '=' << 
		Percent(pubFrozenCount, pubTotalCount) << "% frozen slices)" <<
		endl;

	os << endc;
}
