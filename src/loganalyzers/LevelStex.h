
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_LEVELSTEX_H
#define POLYGRAPH__LOGANALYZERS_LEVELSTEX_H

#include "xstd/String.h"
#include "base/StatIntvlRec.h"
#include "base/LevelStat.h"

// an algorithm of extracting a particular level statistics out of
// interval stats record
class LevelStex {
	public:
		typedef LevelStat StatIntvlRec::*StatPtr;

	public:
		LevelStex(const String &aKey, const String &aName, StatPtr aStats):
			theKey(aKey), theName(aName), theStats(aStats) {}

		const String &key() const { return theKey; } // precise, for machine use
		const String &name() const { return theName; } // imprecise, human-readable

		const LevelStat &level(const StatIntvlRec &rec) const { return rec.*theStats; }

	protected:
		String theKey;
		String theName;
		StatPtr theStats;
};

#endif
