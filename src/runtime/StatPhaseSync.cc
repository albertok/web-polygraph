
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/UniqId.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/StatPhaseSync.h"

class StatPhaseGroup {
	public:
		StatPhaseGroup(const UniqId &aGroup): theGroup(aGroup), thePhaseSyncPos(-1) {}

		const UniqId &group() const { return theGroup; }
		int phaseSyncPos() const { return thePhaseSyncPos; }

		void phaseSyncPos(int aPos) { thePhaseSyncPos = aPos; }

	protected:
		UniqId theGroup;
		int thePhaseSyncPos;
};


StatPhaseSync TheStatPhaseSync;


StatPhaseSync::StatPhaseSync(): theCount(0), thePhaseSyncPosMin(-1) {
	theIndex.resize(37); // XXX: we need to grow this map on-demand basis!
}

int StatPhaseSync::phaseSyncPos() const {
	return Min(thePhaseSyncPosMin, TheStatPhaseMgr.phaseSyncPos());
}

int StatPhaseSync::waitGroupCount() const {
	const int ourPos = TheStatPhaseMgr.phaseSyncPos();
	int cnt = 0;
	for (int i = 0; i < capacity(); ++i) {
		if (StatPhaseGroup *r = theIndex[i]) {
			if (r->phaseSyncPos() < ourPos)
				cnt++;
		}
	}
	return cnt;
}

void StatPhaseSync::notePhaseSync(const UniqId &group, int pos) {
	int idx = -1;
	if (!find(group, idx)) {
		addAt(idx, group);
		Assert(pos >= 0);
		thePhaseSyncPosMin = -1; // will force sync check below
	}
	StatPhaseGroup *grp = theIndex[idx];

	const int oldPos = grp->phaseSyncPos();
	// ignore stale information
	if (pos <= oldPos)
		return;
	grp->phaseSyncPos(pos);

	if (thePhaseSyncPosMin < pos) {
		thePhaseSyncPosMin = -1;
		bool inited = false;
		for (int i = 0; i < capacity(); ++i) {
			if (StatPhaseGroup *r = theIndex[i]) {
				if (!inited || r->phaseSyncPos() < thePhaseSyncPosMin) {
					thePhaseSyncPosMin = r->phaseSyncPos();
					inited = true;
				}
			}
		}
	}

	if (TheStatPhaseMgr->unlockToStop() && TheStatPhaseMgr.phaseSyncPos() <= thePhaseSyncPosMin)
		TheStatPhaseMgr->unlock(StatPhase::ltPhaseSync);
}

bool StatPhaseSync::find(const UniqId &group, int &idx) const {
	if (!capacity())
		return false;

	// start with a hash and then try linear search
	idx = group.hash() % capacity();
	for (int i = capacity(); i; --i) {
		bool res = false;
		if (endSearch(group, idx, res))
			return res;
		idx++;
		idx %= capacity();
	}

	Assert(false); // no empty slots left!
	return false;
}

StatPhaseGroup *StatPhaseSync::addAt(int idx, const UniqId &group) {
	Assert(group);
	Assert(!theIndex[idx]);
	theIndex.put(new StatPhaseGroup(group), idx);
	theCount++;
	return theIndex[idx];
}

// returns true if there is no reason to search further (match or empty)
bool StatPhaseSync::endSearch(const UniqId &group, int idx, bool &res) const {
	if (StatPhaseGroup *h = theIndex[idx]) {
		if (h->group() == group)
			return res = true;
		return res = false;
	}

	// found empty slot
	res = false;
	return true;
}
