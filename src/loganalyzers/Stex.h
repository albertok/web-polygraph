
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_STEX_H
#define POLYGRAPH__LOGANALYZERS_STEX_H

#include "xstd/String.h"
#include "base/StatPhaseRec.h"

class XmlNodes;
class PhaseInfo;


// an algorithm of extracting a particular named statistics out of
// cycle or phase stats record and describing/categorizing that statistics
class Stex {
	public:
		typedef AggrStat StatPhaseRec::*AggrPtr;
		typedef TmSzHistStat StatPhaseRec::*HistPtr;
		typedef ContType::AggrStat StatIntvlRec::*ContTypeAggrPtr;
		typedef ContType::HistStat StatPhaseRec::*ContTypeHistPtr;
		typedef HRHistStat StatPhaseRec::*HRHistPtr;
		typedef CompoundXactStat StatPhaseRec::*CompoundPtr;
		typedef TmSzStat StatIntvlRec::*TracePtr;
		typedef StatusCodeStat StatPhaseRec::*StatusCodePtr;

	public:
		Stex(const String &aKey, const String &aName);
		virtual ~Stex();

		void parent(const Stex *aParent);

		const String &key() const { return theKey; } // precise, for machine use
		const String &name() const { return theName; } // imprecise, human-readable
		const Stex *parent() const { return theParent; }
		bool ignoreUnseen() const { return doIgnoreUnseen; }
		double totalCount(const PhaseInfo &phase) const;
		double meanPartsCount(const PhaseInfo &phase) const;
		int cmpByCountContrib(const PhaseInfo &phase, const Stex &stex) const;

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;
		virtual const TmSzHistStat *hist(const PhaseInfo &) const { return 0; }
		virtual const TmSzStat *trace(const StatIntvlRec &) const { return 0; }
		virtual const Histogram *partsHist(const PhaseInfo &) const { return 0; }
		virtual const AggrStat *partsStat(const PhaseInfo &phase) const;

		virtual void describe(XmlNodes &nodes) const = 0;

	protected:
		void describeParent(XmlNodes &nodes) const;

	protected:
		const String theKey;
		const String theName;
		const Stex *theParent; // if we are a part of a 'larger' stats group
		bool doIgnoreUnseen; // if stex should be added to the unseen stat list
};


class HitsStex: public Stex {
	public:
		HitsStex(const String &aKey, const String &aName);

		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;
		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;
};

class MissesStex: public Stex {
	public:
		MissesStex(const String &aKey, const String &aName);

		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;
		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;
};

class HitMissesStex: public Stex {
	public:
		HitMissesStex(const String &aKey, const String &aName);

		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;
		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

	protected:
		mutable TmSzHistStat theXactHist;
		mutable TmSzStat theXactAggr;

		virtual void describe(XmlNodes &nodes) const;
};


class ValidationHitStex: public Stex {
	public:
		ValidationHitStex(const String &aKey, const String &aName, const HRHistPtr aHRHist);

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;
		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable TmSzStat theXactAggr;

		const HRHistPtr theHRHist;
};

class ValidationMissStex: public Stex {
	public:
		ValidationMissStex(const String &aKey, const String &aName, const HRHistPtr aHRHist);

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;
		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable TmSzStat theXactAggr;

		const HRHistPtr theHRHist;
};

class ValidationHitMissStex: public Stex {
	public:
		ValidationHitMissStex(const String &aKey, const String &aName, const HRHistPtr aHRHist, const TracePtr aTrace);

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;
		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;
		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable TmSzHistStat theXactHist;
		mutable TmSzStat theXactAggr;

		const HRHistPtr theHRHist;
		const TracePtr theTrace;
};


class CachableStex: public Stex {
	public:
		CachableStex(const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;
};

class UnCachableStex: public Stex {
	public:
		UnCachableStex(const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;
};

class AllCachableStex: public Stex {
	public:
		AllCachableStex(const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable TmSzStat theXactAggr;
};


class FillStex: public Stex {
	public:
		FillStex(const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;
};


class SimpleStex: public Stex {
	public:
		SimpleStex(const String &aKey, const String &aName, HistPtr aHist, TracePtr aTrace);

		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;
		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		HistPtr theHist;
		TracePtr theTrace;
};


class AllMethodsStex: public Stex {

	public:
		AllMethodsStex(const String &aKey, const String &aName);

		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;
		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable TmSzHistStat theXactHist;
		mutable TmSzStat theXactAggr;
};


class AllRepsStex: public Stex {
	public:
		AllRepsStex(const String &aKey, const String &aName);

		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;
		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable TmSzHistStat theXactHist;
		mutable TmSzStat theXactAggr;
};


class ContTypeStex: public Stex {
	public:
		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;
		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		ContTypeStex(const String &aKey, const String &aName, const int idx, const ContTypeAggrPtr aContTypeAggr, const ContTypeHistPtr aContTypeHist);

		const int theIdx;
		const ContTypeAggrPtr theContTypeAggr;
		const ContTypeHistPtr theContTypeHist;
};

class RepContTypeStex: public ContTypeStex {
	public:
		RepContTypeStex(const String &aKey, const String &aName, const int idx);
};

class ReqContTypeStex: public ContTypeStex {
	public:
		ReqContTypeStex(const String &aKey, const String &aName, const int idx);
};


class AllContTypesStex: public Stex {
	public:
		virtual const TmSzHistStat *hist(const PhaseInfo &phase) const;
		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		AllContTypesStex(const String &aKey, const String &aName, const ContTypeAggrPtr aContTypeAggr, const ContTypeHistPtr aContTypeHist);

		const ContTypeAggrPtr theContTypeAggr;
		const ContTypeHistPtr theContTypeHist;
		mutable TmSzStat theAggrStat;
		mutable TmSzHistStat theHistStat;
};

class AllRepContTypesStex: public AllContTypesStex {
	public:
		AllRepContTypesStex(const String &aKey, const String &aName);
};

class AllReqContTypesStex: public AllContTypesStex {
	public:
		AllReqContTypesStex(const String &aKey, const String &aName);
};


class CompoundReplyStex: public Stex {
	public:
		CompoundReplyStex(const String &aKey, const String &aName, const CompoundPtr aCompoundPtr);

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;
		virtual const Histogram *partsHist(const PhaseInfo &phase) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		const CompoundPtr theCompoundPtr;
		mutable TmSzStat theStat;
};

class CompoundRequestStex: public Stex {
	public:
		CompoundRequestStex(const String &aKey, const String &aName, const CompoundPtr aCompoundPtr);

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;
		virtual const Histogram *partsHist(const PhaseInfo &phase) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		const CompoundPtr theCompoundPtr;
		mutable TmSzStat theStat;
};

class AllCompoundRepsStex: public Stex {
	public:
		AllCompoundRepsStex(const String &aKey, const String &aName);

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;
		virtual const Histogram *partsHist(const PhaseInfo &phase) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable CompoundXactStat theCompound;
		mutable TmSzStat theStat;
};

class AllCompoundReqsStex: public Stex {
	public:
		AllCompoundReqsStex(const String &aKey, const String &aName);

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;
		virtual const Histogram *partsHist(const PhaseInfo &phase) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable CompoundXactStat theCompound;
		mutable TmSzStat theStat;
};


class AuthIngStex: public Stex {
	public:
		AuthIngStex(const String &aKey, const String &aName, const AuthPhaseStat::Scheme aScheme);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		const AuthPhaseStat::Scheme theScheme;
};

class AuthEdStex: public Stex {
	public:
		AuthEdStex(const String &aKey, const String &aName, const AuthPhaseStat::Scheme aScheme);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		const AuthPhaseStat::Scheme theScheme;
};


class AllAuthIngStex: public Stex {
	public:
		AllAuthIngStex(const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable TmSzStat theStat;
};

class AllAuthEdStex: public Stex {
	public:
		AllAuthEdStex(const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable TmSzStat theStat;
};

class AllAuthStex: public Stex {
	public:
		AllAuthStex(const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		mutable TmSzStat theStat;
};


class ProtoIntvlStex: public Stex {
	public:
		typedef ProtoIntvlStat StatIntvlRec::*ProtoPtr;
		ProtoIntvlStex(ProtoPtr theProto, const String &aKey, const String &aName);

	protected:
		ProtoPtr theProto;
};

class ProtoHitsStex: public ProtoIntvlStex {
	public:
		ProtoHitsStex(ProtoPtr theProto, const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;
};

class ProtoMissesStex: public ProtoIntvlStex {
	public:
		ProtoMissesStex(ProtoPtr theProto, const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

		virtual void describe(XmlNodes &nodes) const;
};

class ProtoHitMissesStex: public ProtoIntvlStex {
	public:
		ProtoHitMissesStex(ProtoPtr theProto, const String &aKey, const String &aName);

		virtual const TmSzStat *trace(const StatIntvlRec &rec) const;

	protected:
		mutable TmSzStat theAggr;

		virtual void describe(XmlNodes &nodes) const;
};

class CookiesStex: public Stex {
	public:
		CookiesStex(const String &aKey, const String &aName, const AggrPtr anAggrPtr);

		virtual const AggrStat *partsStat(const PhaseInfo &phase) const;

	protected:
		const AggrPtr theAggrPtr;

		virtual void describe(XmlNodes &nodes) const;
};

class AllStatusCodeStex: public Stex {
	public:
		AllStatusCodeStex(const String &aKey, const String &aName, const StatusCodePtr aPtr);

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		const StatusCodePtr thePtr;
		mutable TmSzStat theAggr;
};

class StatusCodeStex: public Stex {
	public:
		StatusCodeStex(const String &aKey, const String &aName, const StatusCodePtr aPtr, const int aStatus);

		virtual const TmSzStat *aggr(const PhaseInfo &phase) const;

		virtual void describe(XmlNodes &nodes) const;

	protected:
		const StatusCodePtr thePtr;
		const int theStatus;
};

#endif
