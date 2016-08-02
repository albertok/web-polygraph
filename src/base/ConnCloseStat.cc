
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/ConnCloseStat.h"


// stores stats about one kind of conn close
class ConnCloseStatItem: public LogObj {
	public:
		ConnCloseStatItem(int aKind = -1);

		const TimeHist &ttlHist() const;
		const Log2Hist &useHist() const;

		void record(Time ttl, int useCnt);
		void add(const ConnCloseStatItem &i);

		virtual OLog &store(OLog &ol) const;
		virtual ILog &load(ILog &il);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		TimeHist theTtlH;
		Log2Hist theUseH;
		int theKind;
};


/* ConnCloseStatItem */

ConnCloseStatItem::ConnCloseStatItem(int aKind): 
	theUseH(0, 16*1024-1), theKind(aKind) {
}

const TimeHist &ConnCloseStatItem::ttlHist() const {
	return theTtlH;
}

const Log2Hist &ConnCloseStatItem::useHist() const {
	return theUseH;
}

void ConnCloseStatItem::record(Time ttl, int useCnt) {
	theTtlH.record(ttl);
	theUseH.record(useCnt);
}

void ConnCloseStatItem::add(const ConnCloseStatItem &i) {
	Assert(theKind == i.theKind);
	theTtlH.add(i.theTtlH);
	theUseH.add(i.theUseH);
}

OLog &ConnCloseStatItem::store(OLog &ol) const {
	return ol << theKind << theUseH << theTtlH;
}

ILog &ConnCloseStatItem::load(ILog &il) {
	return il >> theKind >> theUseH >> theTtlH;
}

ostream &ConnCloseStatItem::print(ostream &os, const String &pfx) const {
	const char *kind = 
		(theKind == ConnCloseStat::ckBusy ? "busy" :
		(theKind == ConnCloseStat::ckIdleLocal ? "idle_local" :
		(theKind == ConnCloseStat::ckIdleForeign ? "idle_foreign" :
		"other")));

	theUseH.print(os, pfx + kind + ".use.");
	theTtlH.print(os, pfx + kind + ".ttl.");
	return os;
}

/* ConnCloseStat */

ConnCloseStat::ConnCloseStat() {
	theStats.stretch((int)ckEnd);
	for (int i = 0; i < (int)ckEnd; ++i)
		theStats.append(new ConnCloseStatItem(i));
}

ConnCloseStat::~ConnCloseStat() {
	while (theStats.count()) delete theStats.pop();
}

void ConnCloseStat::useHist(Log2Hist &acc) const {
	for (int i = 0; i < theStats.count(); ++i)
		acc.add(useHist(i));
}

void ConnCloseStat::ttlHist(TimeHist &acc) const {
	for (int i = 0; i < theStats.count(); ++i)
		acc.add(ttlHist(i));
}

const Log2Hist &ConnCloseStat::useHist(int ck) const {
	Assert(0 <= ck && ck < theStats.count());
	return theStats[ck]->useHist();
}

const TimeHist &ConnCloseStat::ttlHist(int ck) const {
	Assert(0 <= ck && ck < theStats.count());
	return theStats[ck]->ttlHist();
}

void ConnCloseStat::record(CloseKind ck, Time ttl, int useCnt) {
	Assert(0 <= ck && ck < theStats.count());
	theStats[(int)ck]->record(ttl, useCnt);
}

void ConnCloseStat::add(const ConnCloseStat &s) {
	Assert(theStats.count() == s.theStats.count());
	for (int i = 0; i < theStats.count(); ++i)
		theStats[i]->add(*s.theStats[i]);
}

OLog &ConnCloseStat::store(OLog &ol) const {
	return ol << theStats;
}

ILog &ConnCloseStat::load(ILog &il) {
	return il >> theStats;
}

ostream &ConnCloseStat::print(ostream &os, const String &pfx) const {
	os << pfx << "categories:\t " << theStats.count() << endl;
	for (int i = 0; i < theStats.count(); ++i)
		theStats[i]->print(os, pfx);
	return os;
}
