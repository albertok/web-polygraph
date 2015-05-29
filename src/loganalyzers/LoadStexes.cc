
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

//#include "xml/XmlText.h"
//#include "xml/XmlParagraph.h"
#include "loganalyzers/Stex.h"
#include "loganalyzers/ReportBlob.h"
#include "loganalyzers/PhaseInfo.h"
#include "loganalyzers/LoadStexes.h"

/* SideLoadStex */

SideLoadStex::SideLoadStex(const String &aKey, const String &aName, StatPtr aRate, StatPtr aBwidth):
	LoadStex(aKey, aName), theRateStats(aRate), theBwidthStats(aBwidth) {
}

double SideLoadStex::rate(const StatIntvlRec &rec) const {
	return (rec.*theRateStats)();
}

double SideLoadStex::bwidth(const StatIntvlRec &rec) const {
	return (rec.*theBwidthStats)();
}


/* TmSzLoadStex */

TmSzLoadStex::TmSzLoadStex(const Stex *aStex):
	LoadStex(aStex->key(), aStex->name()), theStex(aStex) {
}

double TmSzLoadStex::rate(const StatIntvlRec &rec) const {
	return perDuration(theStex->trace(rec)->count(), rec);
}

double TmSzLoadStex::bwidth(const StatIntvlRec &rec) const {
	return perDuration(theStex->trace(rec)->size().sum(), rec);
}


/* ProtoSideLoadStex */

ProtoSideLoadStex::ProtoSideLoadStex(const String &aKey, const String &aName,
	ProtoPtr aProto, StatPtr aRate, StatPtr aBwidth):
	LoadStex(aKey, aName), theProto(aProto),
	theRateStats(aRate), theBwidthStats(aBwidth) {
}

double ProtoSideLoadStex::rate(const StatIntvlRec &rec) const {
	return (rec.*theProto.*theRateStats)(rec.theDuration);
}

double ProtoSideLoadStex::bwidth(const StatIntvlRec &rec) const {
	return (rec.*theProto.*theBwidthStats)(rec.theDuration);
}
