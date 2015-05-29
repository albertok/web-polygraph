
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGEXTRACTORS_AGGRINFOLOADER_H
#define POLYGRAPH__LOGEXTRACTORS_AGGRINFOLOADER_H

#include "base/StatPhaseRec.h"

class PhaseGroup: public StatPhaseRec {
	public:
		PhaseGroup(const String &aName);
};

class AggrInfoLoader {
	protected:
		typedef const Array<String*> &Strs;

	public:
		AggrInfoLoader(Strs phaseNames);
		virtual ~AggrInfoLoader();

		void filter(LogCatFilter *aFilter) { theFilter = aFilter; }

		bool load(ILog &il, const LogEntryPx &px);

		int groupCount() const;
		const PhaseGroup *group(const String &name) const;
		const PhaseGroup *group(int idx) const;
		int aggregate(StatPhaseRec &r) const;

	protected:
		virtual void notePhase(const StatPhaseRec &ph);
		void addCat(int cat);

	protected:
		Strs thePhaseNames;

		Array<PhaseGroup*> theGroups;
		LogCatFilter *theFilter;
};

#endif
