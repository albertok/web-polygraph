
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_STATPHASESYNC_H
#define POLYGRAPH__RUNTIME_STATPHASESYNC_H

#include "xstd/Array.h"

class UniqId;
class StatPhase;
class StatPhaseGroup;

// a GroupId <-> PhaseGroup map with phase synchronization info
// add-only operation
class StatPhaseSync {
	public:
		StatPhaseSync();

		int capacity() const { return theIndex.capacity(); }
		Counter count() const { return theCount; }

		int phaseSyncPosMin() const { return thePhaseSyncPosMin; }
		int phaseSyncPos() const;
		int waitGroupCount() const;

		void notePhaseSync(const UniqId &grp, int pos);

	protected:
		bool find(const UniqId &group, int &idx) const;
		StatPhaseGroup *addAt(int idx, const UniqId &group);
		bool endSearch(const UniqId &group, int idx, bool &res) const;

	protected:
		PtrArray<StatPhaseGroup*> theIndex;
		Counter theCount; // entries in the index

		int thePhaseSyncPosMin; // min among all known groups
};

extern StatPhaseSync TheStatPhaseSync;

#endif
