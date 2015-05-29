
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_POINTSTEX_H
#define POLYGRAPH__LOGANALYZERS_POINTSTEX_H

#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "base/StatIntvlRec.h"
#include "loganalyzers/StexBase.h"

// an algorithm of extracting a single value statistics out of
// interval stats record
class PointStex: public StexBase<double, StatIntvlRec> {
	public:
		PointStex(const String &aKey, const String &aName, const String &aUnit):
			StexBase<double,StatIntvlRec>(aKey, aName, aUnit) {}

		virtual bool valueKnown(const StatIntvlRec &rec) const = 0;
		virtual double value(const StatIntvlRec &rec) const = 0;
};

class AggrPointStex: public PointStex {
	public:
		typedef AggrStat StatIntvlRec::*AggrPtr;

	public:
		AggrPointStex(const String &aKey, const String &aName, const String &aUnit,
			AggrPtr anAggrPtr): PointStex(aKey, aName, aUnit), 
			theAggrPtr(anAggrPtr) {}

		const AggrStat &aggr(const StatIntvlRec &rec) const {
			return rec.*theAggrPtr; }

		virtual bool valueKnown(const StatIntvlRec &rec) const {
			return (rec.*theAggrPtr).known(); }

	protected:
		AggrPtr theAggrPtr;
};

class HRPointStex: public PointStex {
	public:
		typedef HRStat StatIntvlRec::*HRPtr;

	public:
		HRPointStex(const String &aKey, const String &aName, const String &aUnit,
			HRPtr anHRPtr): PointStex(aKey, aName, aUnit), 
			theHRPtr(anHRPtr) {}

		const HRStat &stats(const StatIntvlRec &rec) const {
			return rec.*theHRPtr; }

		virtual bool valueKnown(const StatIntvlRec &rec) const {
			return stats(rec).active(); }

	protected:
		HRPtr theHRPtr;
};

class MeanAggrPointStex: public AggrPointStex {
	public:
		MeanAggrPointStex(const String &aKey, const String &aName, const String &aUnit,
			AggrPtr anAggrPtr): AggrPointStex(aKey, aName, aUnit, anAggrPtr){}

		virtual double value(const StatIntvlRec &rec) const {
			return aggr(rec).mean(); }
};

class DhpPointStex: public HRPointStex {
	public:
		DhpPointStex(const String &aKey, const String &aName,
			HRPtr anHRPtr): HRPointStex(aKey, aName, "%", anHRPtr){}

		virtual double value(const StatIntvlRec &rec) const {
			return stats(rec).dhp(); }
};

class BhpPointStex: public HRPointStex {
	public:
		BhpPointStex(const String &aKey, const String &aName,
			HRPtr anHRPtr): HRPointStex(aKey, aName, "%", anHRPtr){}

		virtual double value(const StatIntvlRec &rec) const {
			return stats(rec).bhp(); }
};

class LoadPointStex: public PointStex {
	public:
		typedef double (StatIntvlRec::*StatPtr)() const;

	public:
		LoadPointStex(const String &aKey, const String &aName, const String &aUnit,
			StatPtr aStatPtr): PointStex(aKey, aName, aUnit),
			theStatPtr(aStatPtr) {}

		virtual bool valueKnown(const StatIntvlRec &rec) const {
			return rec.theDuration > 0; }

		virtual double value(const StatIntvlRec &rec) const {
			return (rec.*theStatPtr)(); }

	protected:
		StatPtr theStatPtr;
};

class PipelineProbPointStex: public PointStex {
	public:
		PipelineProbPointStex(): PointStex("pprob", 
			"portion of pipelined connections", "%") {}

		virtual bool valueKnown(const StatIntvlRec &rec) const {
			return rec.theConnUseCnt.known(); }

		virtual double value(const StatIntvlRec &rec) const {
			return Percent(rec.theConnPipelineDepth.count(),
				rec.theConnUseCnt.count());
		}
};

class MeanRptmPointStex: public PointStex {
	public:
		MeanRptmPointStex(): PointStex("mean-rptm", 
			"mean response time", "msec") {}

		virtual bool valueKnown(const StatIntvlRec &rec) const {
			return rec.repTime().known(); }

		virtual double value(const StatIntvlRec &rec) const {
			return rec.repTime().mean(); }
};

#endif
