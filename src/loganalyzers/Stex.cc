
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xml/XmlText.h"
#include "xml/XmlParagraph.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/Stex.h"


Stex::Stex(const String &aKey, const String &aName):
	theKey(aKey), theName(aName),
	theParent(0), doIgnoreUnseen(false) {
}

Stex::~Stex() {
}

void Stex::parent(const Stex *aParent) {
	Assert(!theParent || !aParent);
	theParent = aParent;
}

double Stex::totalCount(const PhaseInfo &phase) const {
	if (const TmSzStat *const recStats = aggr(phase))
		return recStats->size().count();

	return 0;
}

double Stex::meanPartsCount(const PhaseInfo &phase) const {
	const AggrStat *const stats = partsStat(phase);
	return stats && stats->known() ? stats->mean() : 1;
}

// compare using "contribution by count" and other factors
int Stex::cmpByCountContrib(const PhaseInfo &phase, const Stex &stex) const {
	static const double epsilon = 1e-3;

	const double x = totalCount(phase) * meanPartsCount(phase);
	const double y = stex.totalCount(phase) * stex.meanPartsCount(phase);
	const double diff = x - y;

	if (diff < -epsilon)
		return -1;
	if (diff > epsilon)
		return 1;

	// check if one stex is parent of another
	// parent stex contribute more than child
	if (parent() == &stex)
		return -1;
	if (stex.parent() == this)
		return 1;

	// compare names if nothing else
	return name().cmp(stex.name());
}

const TmSzStat *Stex::aggr(const PhaseInfo &phase) const {
	return trace(phase.availStats());
}

const AggrStat *Stex::partsStat(const PhaseInfo &phase) const {
	const Histogram *const h = partsHist(phase);
	return h ? &h->stats() : 0;
}

void Stex::describe(XmlNodes &nodes) const {
	nodes << XmlTextTag<XmlParagraph>("No description is available for "
		"this object class.");
	describeParent(nodes);
}

void Stex::describeParent(XmlNodes &nodes) const {
	if (parent()) {
		XmlTextTag<XmlParagraph> text;
		text.buf() << "This object class belongs to the '"
			<< parent()->name() << "' class.";
		nodes << text;
	}
}


/* HitsStex */

HitsStex::HitsStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzHistStat *HitsStex::hist(const PhaseInfo &phase) const {
	return phase.hasStats() ? &phase.stats().theBasicXacts.hits() : 0;
}

const TmSzStat *HitsStex::trace(const StatIntvlRec &rec) const {
	return &rec.theRealHR.hits();
}

void HitsStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* MissesStex */

MissesStex::MissesStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzHistStat *MissesStex::hist(const PhaseInfo &phase) const {
	return phase.hasStats() ? &phase.stats().theBasicXacts.misses() : 0;
}

const TmSzStat *MissesStex::trace(const StatIntvlRec &rec) const {
	return &rec.theRealHR.misses();
}

void MissesStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* HitMissesStex */

HitMissesStex::HitMissesStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzHistStat *HitMissesStex::hist(const PhaseInfo &phase) const {
	if (phase.hasStats()) {
		theXactHist.reset();
		theXactHist = phase.stats().theBasicXacts.hits();
		theXactHist += phase.stats().theBasicXacts.misses();
		return &theXactHist;
	} else {
		return 0;
	}
}

const TmSzStat *HitMissesStex::trace(const StatIntvlRec &rec) const {
	theXactAggr = rec.theRealHR.xacts();
	return &theXactAggr;
}

void HitMissesStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* ValidationHitStex */

ValidationHitStex::ValidationHitStex(const String &aKey, const String &aName, const HRHistPtr aHRHist):
	Stex(aKey, aName), theHRHist(aHRHist) {
}

const TmSzStat *ValidationHitStex::aggr(const PhaseInfo &phase) const {
	if (phase.hasStats()) {
		theXactAggr = (phase.stats().*theHRHist).hits().aggr();
		return &theXactAggr;
	} else {
		return 0;
	}
}

const TmSzHistStat *ValidationHitStex::hist(const PhaseInfo &phase) const {
	return phase.hasStats() ? &(phase.stats().*theHRHist).hits() : 0;
}

void ValidationHitStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* ValidationMissStex */

ValidationMissStex::ValidationMissStex(const String &aKey, const String &aName, const HRHistPtr aHRHist):
	Stex(aKey, aName), theHRHist(aHRHist) {
}

const TmSzStat *ValidationMissStex::aggr(const PhaseInfo &phase) const {
	if (phase.hasStats()) {
		theXactAggr = (phase.stats().*theHRHist).misses().aggr();
		return &theXactAggr;
	} else {
		return 0;
	}
}

const TmSzHistStat *ValidationMissStex::hist(const PhaseInfo &phase) const {
	return phase.hasStats() ? &(phase.stats().*theHRHist).misses() : 0;
}

void ValidationMissStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* ImsStex */

ValidationHitMissStex::ValidationHitMissStex(const String &aKey, const String &aName, const HRHistPtr aHRHist, const TracePtr aTrace):
	Stex(aKey, aName), theHRHist(aHRHist), theTrace(aTrace) {
}

const TmSzStat *ValidationHitMissStex::aggr(const PhaseInfo &phase) const {
	const TmSzStat *stat;
	if (theTrace)
		stat = Stex::aggr(phase);
	else
	if (phase.hasStats()) {
		theXactAggr.reset();
		theXactAggr += (phase.stats().*theHRHist).hits().aggr();
		theXactAggr += (phase.stats().*theHRHist).misses().aggr();
		stat = &theXactAggr;
	} else
		stat = 0;
	return stat;
}

const TmSzHistStat *ValidationHitMissStex::hist(const PhaseInfo &phase) const {
	if (phase.hasStats()) {
		theXactHist.reset();
		theXactHist = (phase.stats().*theHRHist).hits();
		theXactHist += (phase.stats().*theHRHist).misses();
		return &theXactHist;
	} else {
		return 0;
	}
}

const TmSzStat *ValidationHitMissStex::trace(const StatIntvlRec &rec) const {
	return theTrace ? &(rec.*theTrace) : 0;
}

void ValidationHitMissStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* CachableStex */

CachableStex::CachableStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzStat *CachableStex::trace(const StatIntvlRec &rec) const {
	return &rec.theChbR.hits();
}

void CachableStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* UnCachableStex */

UnCachableStex::UnCachableStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzStat *UnCachableStex::trace(const StatIntvlRec &rec) const {
	return &rec.theChbR.misses();
}

void UnCachableStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AllCachableStex */

AllCachableStex::AllCachableStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzStat *AllCachableStex::trace(const StatIntvlRec &rec) const {
	theXactAggr = rec.theChbR.xacts();
	return &theXactAggr;
}

void AllCachableStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* FillStex */

FillStex::FillStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzStat *FillStex::trace(const StatIntvlRec &rec) const {
	return &rec.theFill;
}

void FillStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* SimpleStex */

SimpleStex::SimpleStex(const String &aKey, const String &aName, HistPtr aHist, TracePtr aTrace):
	Stex(aKey, aName), theHist(aHist), theTrace(aTrace) {
}

const TmSzHistStat *SimpleStex::hist(const PhaseInfo &phase) const {
	return theHist && phase.hasStats() ? &(phase.stats().*theHist) : 0;
}

const TmSzStat *SimpleStex::trace(const StatIntvlRec &rec) const {
	return theTrace ? &(rec.*theTrace) : 0;
}

void SimpleStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AllMethodsStex */

AllMethodsStex::AllMethodsStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzHistStat *AllMethodsStex::hist(const PhaseInfo &phase) const {
	if (phase.hasStats()) {
		theXactHist.reset();
		theXactHist += phase.stats().theHeadXacts;
		theXactHist += phase.stats().thePostXacts;
		theXactHist += phase.stats().thePutXacts;
		theXactHist += phase.stats().theConnectXacts;
		return &theXactHist;
	} else {
		return 0;
	}
}

const TmSzStat *AllMethodsStex::trace(const StatIntvlRec &rec) const {
	theXactAggr = rec.theHead + rec.thePost + rec.thePut + rec.theConnectStat.doneXacts().misses();
	return &theXactAggr;
}

void AllMethodsStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AllRepsStex */

AllRepsStex::AllRepsStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzHistStat *AllRepsStex::hist(const PhaseInfo &phase) const {
	if (phase.hasStats()) {
		theXactHist.reset();
		phase.stats().repAll(theXactHist);
		return &theXactHist;
	} else {
		return 0;
	}
}

const TmSzStat *AllRepsStex::trace(const StatIntvlRec &rec) const {
	theXactAggr = rec.reps();
	return &theXactAggr;
}

void AllRepsStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* ContTypeStex */

ContTypeStex::ContTypeStex(const String &aKey, const String &aName, const int anIdx, const ContTypeAggrPtr aContTypeAggr, const ContTypeHistPtr aContTypeHist):
	Stex(aKey, aName), theIdx(anIdx), theContTypeAggr(aContTypeAggr),
	theContTypeHist(aContTypeHist) {
	Must(theContTypeAggr);
	Must(theContTypeHist);
}

const TmSzHistStat *ContTypeStex::hist(const PhaseInfo &phase) const {
	return phase.hasStats() ?
		(phase.stats().*theContTypeHist).hasStats(theIdx) : 0;
}

const TmSzStat *ContTypeStex::trace(const StatIntvlRec &rec) const {
	static const TmSzStat emptyStats;
	const TmSzStat *const stats = (rec.*theContTypeAggr).hasStats(theIdx);
	return stats ? stats : &emptyStats;
}

void ContTypeStex::describe(XmlNodes &nodes) const {
	XmlText text;
	if (theIdx < ContType::NormalContentStart()) {
		text.buf() << "The " << name() << " object represents one of "
			"the built-in content kinds used by Polygraph for "
			"messages that do not match any of the PGL-configured "
			"Content kinds.";
		// TODO: Also document this specific built-in kind?
	} else {
		text.buf() << "This object class represents the " << name() <<
			" Content object in the PGL workload.";
	}
	nodes << text;
}


/* RepContTypeStex */

RepContTypeStex::RepContTypeStex(const String &aKey, const String &aName, const int idx):
	ContTypeStex(aKey, aName, idx, &StatIntvlRec::theRepContType, &StatPhaseRec::theRepContTypeHist) {
}


/* ReqContTypeStex */

ReqContTypeStex::ReqContTypeStex(const String &aKey, const String &aName, const int idx):
	ContTypeStex(aKey, aName, idx, &StatIntvlRec::theReqContType, &StatPhaseRec::theReqContTypeHist) {
}


/* AllContTypesStex */

AllContTypesStex::AllContTypesStex(const String &aKey, const String &aName, const ContTypeAggrPtr aContTypeAggr, const ContTypeHistPtr aContTypeHist):
	Stex(aKey, aName), theContTypeAggr(aContTypeAggr),
	theContTypeHist(aContTypeHist) {
	Must(theContTypeAggr);
	Must(theContTypeHist);
}

const TmSzHistStat *AllContTypesStex::hist(const PhaseInfo &phase) const {
	const StatPhaseRec *const rec = phase.hasStats();
	if (rec) {
		theHistStat.reset();
		for (int i = 0; i < ContType::Count(); ++i) {
			if ((rec->*theContTypeHist).hasStats(i))
				theHistStat += (rec->*theContTypeHist).stats(i);
		}
		return &theHistStat;
	} else
		return 0;
}

const TmSzStat *AllContTypesStex::trace(const StatIntvlRec &rec) const {
	theAggrStat.reset();
	for (int i = 0; i < ContType::Count(); ++i) {
		if ((rec.*theContTypeAggr).hasStats(i))
			theAggrStat += (rec.*theContTypeAggr).stats(i);
	}
	return &theAggrStat;
}

void AllContTypesStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AllRepContTypeStex */

AllRepContTypesStex::AllRepContTypesStex(const String &aKey, const String &aName):
	AllContTypesStex(aKey, aName, &StatIntvlRec::theRepContType, &StatPhaseRec::theRepContTypeHist) {
}


/* AllReqContTypeStex */

AllReqContTypesStex::AllReqContTypesStex(const String &aKey, const String &aName):
	AllContTypesStex(aKey, aName, &StatIntvlRec::theReqContType, &StatPhaseRec::theReqContTypeHist) {
}


/* CompoundReplyStex */

CompoundReplyStex::CompoundReplyStex(const String &aKey, const String &aName, const CompoundPtr aCompoundPtr):
	Stex(aKey, aName), theCompoundPtr(aCompoundPtr) {
}

const TmSzStat *CompoundReplyStex::aggr(const PhaseInfo &phase) const {
	const CompoundXactStat &compound = phase.stats().*theCompoundPtr;
	theStat = TmSzStat(compound.duration.stats(), compound.repSize.stats());
	return &theStat;
}

const Histogram *CompoundReplyStex::partsHist(const PhaseInfo &phase) const {
	const CompoundXactStat &compound = phase.stats().*theCompoundPtr;
	return &compound.exchanges;
}

void CompoundReplyStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* CompoundRequestStex */

CompoundRequestStex::CompoundRequestStex(const String &aKey, const String &aName, const CompoundPtr aCompoundPtr):
	Stex(aKey, aName), theCompoundPtr(aCompoundPtr) {
}

const TmSzStat *CompoundRequestStex::aggr(const PhaseInfo &phase) const {
	const CompoundXactStat &compound = phase.stats().*theCompoundPtr;
	theStat = TmSzStat(compound.duration.stats(), compound.reqSize.stats());
	return &theStat;
}

const Histogram *CompoundRequestStex::partsHist(const PhaseInfo &phase) const {
	const CompoundXactStat &compound = phase.stats().*theCompoundPtr;
	return &compound.exchanges;
}

void CompoundRequestStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}

/* AllCompoundRepsStex */

AllCompoundRepsStex::AllCompoundRepsStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzStat *AllCompoundRepsStex::aggr(const PhaseInfo &phase) const {
	theCompound.reset();
	phase.stats().compoundAll(theCompound);
	theStat = TmSzStat(theCompound.duration.stats(), theCompound.repSize.stats());
	return &theStat;
}

const Histogram *AllCompoundRepsStex::partsHist(const PhaseInfo &phase) const {
	theCompound.reset();
	phase.stats().compoundAll(theCompound);
	return &theCompound.exchanges;
}

void AllCompoundRepsStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}

/* AllCompoundReqsStex */

AllCompoundReqsStex::AllCompoundReqsStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzStat *AllCompoundReqsStex::aggr(const PhaseInfo &phase) const {
	theCompound.reset();
	phase.stats().compoundAll(theCompound);
	theStat = TmSzStat(theCompound.duration.stats(), theCompound.reqSize.stats());
	return &theStat;
}

const Histogram *AllCompoundReqsStex::partsHist(const PhaseInfo &phase) const {
	theCompound.reset();
	phase.stats().compoundAll(theCompound);
	return &theCompound.exchanges;
}

void AllCompoundReqsStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AuthIngStex */

AuthIngStex::AuthIngStex(const String &aKey, const String &aName, const AuthPhaseStat::Scheme aScheme):
	Stex(aKey, aName), theScheme(aScheme) {
}

const TmSzStat *AuthIngStex::trace(const StatIntvlRec &rec) const {
	return &rec.theAuth.getAuthIng(theScheme);
}

void AuthIngStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AuthEdStex */

AuthEdStex::AuthEdStex(const String &aKey, const String &aName, const AuthPhaseStat::Scheme aScheme):
	Stex(aKey, aName), theScheme(aScheme) {
}

const TmSzStat *AuthEdStex::trace(const StatIntvlRec &rec) const {
	return &rec.theAuth.getAuthEd(theScheme);
}

void AuthEdStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AllAuthIngStex */

AllAuthIngStex::AllAuthIngStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzStat *AllAuthIngStex::trace(const StatIntvlRec &rec) const {
	theStat.reset();
	rec.theAuth.authIngAll(theStat);
	return &theStat;
}

void AllAuthIngStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AllAuthEdStex */

AllAuthEdStex::AllAuthEdStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzStat *AllAuthEdStex::trace(const StatIntvlRec &rec) const {
	theStat.reset();
	rec.theAuth.authEdAll(theStat);
	return &theStat;
}

void AllAuthEdStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AllAuthStex */

AllAuthStex::AllAuthStex(const String &aKey, const String &aName):
	Stex(aKey, aName) {
}

const TmSzStat *AllAuthStex::trace(const StatIntvlRec &rec) const {
	theStat.reset();
	rec.theAuth.authIngAll(theStat);
	rec.theAuth.authEdAll(theStat);
	return &theStat;
}

void AllAuthStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* ProtoIntvlStex */

ProtoIntvlStex::ProtoIntvlStex(ProtoPtr aProto, const String &aKey,
	const String &aName): Stex(aKey, aName), theProto(aProto) {
}

/* ProtoHitsStex */

ProtoHitsStex::ProtoHitsStex(ProtoPtr aProto, const String &aKey, const String &aName):
	ProtoIntvlStex(aProto, aKey, aName) {
}

const TmSzStat *ProtoHitsStex::trace(const StatIntvlRec &rec) const {
	return &(rec.*theProto).doneXacts().hits();
}

void ProtoHitsStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* ProtoMissesStex */

ProtoMissesStex::ProtoMissesStex(ProtoPtr aProto, const String &aKey, const String &aName):
	ProtoIntvlStex(aProto, aKey, aName) {
}

const TmSzStat *ProtoMissesStex::trace(const StatIntvlRec &rec) const {
	return &(rec.*theProto).doneXacts().misses();
}

void ProtoMissesStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* ProtoHitMissesStex */

ProtoHitMissesStex::ProtoHitMissesStex(ProtoPtr aProto, const String &aKey, const String &aName):
	ProtoIntvlStex(aProto, aKey, aName) {
}

const TmSzStat *ProtoHitMissesStex::trace(const StatIntvlRec &rec) const {
	theAggr = (rec.*theProto).doneXacts().xacts();
	return &theAggr;
}

void ProtoHitMissesStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* CookiesStex */

CookiesStex::CookiesStex(const String &aKey, const String &aName, const AggrPtr anAggrPtr):
	Stex(aKey, aName), theAggrPtr(anAggrPtr) {
}

const AggrStat *CookiesStex::partsStat(const PhaseInfo &phase) const {
	return &(phase.stats().*theAggrPtr);
}

void CookiesStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* AllStatusCodeStex */

AllStatusCodeStex::AllStatusCodeStex(const String &aKey, const String &aName, const StatusCodePtr aPtr):
	Stex(aKey, aName), thePtr(aPtr) {
	Assert(thePtr);
	doIgnoreUnseen = true;
}

const TmSzStat *AllStatusCodeStex::aggr(const PhaseInfo &phase) const {
	theAggr = (phase.stats().*thePtr).allStats();
	return &theAggr;
}

void AllStatusCodeStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}


/* StatusCodeStex */

StatusCodeStex::StatusCodeStex(const String &aKey, const String &aName, const StatusCodePtr aPtr, const int aStatus):
	Stex(aKey, aName), thePtr(aPtr), theStatus(aStatus) {
	Assert(thePtr);
	doIgnoreUnseen = true;
}

const TmSzStat *StatusCodeStex::aggr(const PhaseInfo &phase) const {
	return (phase.stats().*thePtr).stats(theStatus);
}

void StatusCodeStex::describe(XmlNodes &nodes) const {
	Stex::describe(nodes);
}
