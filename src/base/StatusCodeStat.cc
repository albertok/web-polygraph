
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/StatusCodeStat.h"

#include "base/AnyToString.h"
#include "base/TmSzStat.h"

StatusCodeStat::StatusCodeStat(): theStats(scsOther + 1) {
	while(!theStats.full()) theStats.append(0);
}

StatusCodeStat::~StatusCodeStat() {
	while (theStats.count()) delete theStats.pop();
}

TmSzStat StatusCodeStat::allStats() const {
	TmSzStat stats;
	for (int i = 0; i < theStats.count(); ++i) {
		if (theStats[i])
			stats += *theStats[i];
	}
	return stats;
}

const TmSzStat *StatusCodeStat::stats(int status) const {
	Assert(scsMinValue <= status && status <= scsMaxValue);

	if (status == scsMinValue)
		status = scsOther;

	return theStats[status];
}

void StatusCodeStat::record(int status, const Time &tm, Size sz) {
	Assert(-1 <= status && status <= 999);

	if (status == scsMinValue)
		status = scsOther;

	if (!theStats[status])
		theStats[status] = new TmSzStat;
	theStats[status]->record(tm, sz);
}

void StatusCodeStat::add(const StatusCodeStat &s) {
	for (int i = 0; i < theStats.count(); ++i) {
		if (s.theStats[i]) {
			if (!theStats[i])
				theStats[i] = new TmSzStat(*s.theStats[i]);
			else
				*theStats[i] += *s.theStats[i];
		}
	}
}

OLog &StatusCodeStat::store(OLog &ol) const {
	for (int i = 0; i < theStats.count(); ++i) {
		if (theStats[i]) {
			ol.puti(i);
			ol << *theStats[i];
		}
	}
	ol.puti(-1);
	return ol;
}

ILog &StatusCodeStat::load(ILog &il) {
	for (int i = 0; i < theStats.count(); ++i) {
		delete theStats[i];
		theStats[i] = 0;
	}
	for (int i = il.geti(); i >= 0; i = il.geti()) {
		Assert(i < theStats.count());
		Assert(!theStats[i]);
		theStats[i] = new TmSzStat;
		il >> *theStats[i];
	}
	return il;
}

ostream &StatusCodeStat::print(ostream &os, const String &pfx) const {
	for (int i = 0; i <= scsOther; ++i) {
		if (theStats[i]) {
			static const String scsOtherLabel("other");
			const String label(i == scsOther ? scsOtherLabel : AnyToString(i));
			theStats[i]->print(os, pfx + label + '.');
		}
	}

	return os;
}
