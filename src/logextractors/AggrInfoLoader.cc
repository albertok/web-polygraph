
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "logextractors/LogIter.h"
#include "logextractors/LogCatFilter.h"
#include "logextractors/AggrInfoLoader.h"


/* PhaseGroup */

PhaseGroup::PhaseGroup(const String &aName) {
	theName = aName;
}


/* AggrInfoLoader */

AggrInfoLoader::AggrInfoLoader(Strs phaseNames):
	thePhaseNames(phaseNames), theFilter(0) {
}

AggrInfoLoader::~AggrInfoLoader() {
	while (theGroups.count()) delete theGroups.pop();
}

bool AggrInfoLoader::load(ILog &il, const LogEntryPx &px) {

	if (!theFilter->passed(px))
		return true;

	switch (px.theTag) {
		case lgContTypeKinds: {
			// should be called only once per log
			ContType::Load(il);
			break;
		}
		/*case lgCltState: {
			// this code allows extraction of robot's private worlds
			int seqvId;
			Array<ObjWorld> privWorlds;
			il >> seqvId >> privWorlds;
			cerr << here << seqvId << ": loaded " << privWorlds.count() << " worlds" << endl;
			static int count = 1250;
			if (count-- > 0)
				break;
			for (int i = 0; i < privWorlds.count(); ++i)
				if (privWorlds[i].wss() == -1)
					cerr << "\t" << seqvId << "." << i << ": " << privWorlds[i] << endl;
			break;
		}*/
		case lgStatPhaseRec: {
			StatPhaseRec ph;
			ph.load(il);
			notePhase(ph);
			break;
		}
	}
	return true;
}

void AggrInfoLoader::notePhase(const StatPhaseRec &ph) {
	// check if we need this phase
	bool found = !thePhaseNames.count();
	{for (int i = 0; !found && i < thePhaseNames.count(); ++i)
		found = ph.name()== *thePhaseNames[i];
	}
	if (!found)
		return;

	// check if we saw this phase before
	PhaseGroup *group = 0;
	{for (int i = 0; !group && i < theGroups.count(); ++i) {
		if (theGroups[i]->name() == ph.name())
			group = theGroups[i];
	}}

	if (!group) {
		group = new PhaseGroup(ph.name());
		theGroups.append(group);
	}

	group->merge(ph);
}

int AggrInfoLoader::groupCount() const {
	return theGroups.count();
}

const PhaseGroup *AggrInfoLoader::group(const String &name) const {
	PhaseGroup *group = 0;
	for (int i = 0; !group && i < theGroups.count(); ++i) {
		if (theGroups[i]->name() == name)
			group = theGroups[i];
	}
	return group;
}

const PhaseGroup *AggrInfoLoader::group(int idx) const {
	Assert(0 <= idx && idx < theGroups.count());
	return theGroups[idx];
}

int AggrInfoLoader::aggregate(StatPhaseRec &r) const {
	// concatenate all filtered phases
	for (int i = 0; i < theGroups.count(); ++i)
		r.concat(*theGroups[i]);

	return groupCount();
}

