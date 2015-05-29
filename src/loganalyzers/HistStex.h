
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_HISTSTEX_H
#define POLYGRAPH__LOGANALYZERS_HISTSTEX_H

#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "base/StatPhaseRec.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/StexBase.h"

// an algorithm of extracting a single value statistics out of
// interval stats record
class HistStex: public StexBase<const Histogram*, PhaseInfo> {
	public:
		typedef TimeHist StatPhaseRec::*TimeHistPtr;
		typedef CompoundXactStat StatPhaseRec::*CompoundXactPtr;

		HistStex(const String &aKey, const String &aName, const String &aUnit):
			StexBase<const Histogram*, PhaseInfo>(aKey, aName, aUnit) {}

		virtual bool valueKnown(const PhaseInfo &phase) const { return value(phase)->known(); }
		virtual const Histogram *value(const PhaseInfo &phase) const = 0;
};

class PipelineDepthHistStex: public HistStex {
	public:
		PipelineDepthHistStex(): HistStex("pdepths", 
			"pipeline depth", "xact") {}

		virtual const Histogram *value(const PhaseInfo &phase) const {
			return &phase.stats().theConnPipelineDepths;
		}
};

class TimeHistStex: public HistStex {
	public:
		TimeHistStex(const String &aKey, const String &aName, const TimeHistPtr aTimeHist):
			HistStex(aKey, aName, "msec"), theTimeHist(aTimeHist) {}

		virtual const Histogram *value(const PhaseInfo &phase) const {
			return &(phase.stats().*theTimeHist);
		}

	protected:

		const TimeHistPtr theTimeHist;
};

template < class T, T CompoundXactStat::*histPtr >
class CompoundXactHistStex: public HistStex {
	public:
		CompoundXactHistStex(const String &aName, const String &aUnit, const CompoundXactPtr aCXactPtr):
			HistStex("cxact", aName, aUnit), theCXactPtr(aCXactPtr) {}

		virtual const Histogram *value(const PhaseInfo &phase) const {
			return &(phase.stats().*theCXactPtr.*histPtr);
		}

	protected:

		const CompoundXactPtr theCXactPtr;
};

#endif
