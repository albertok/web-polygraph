
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"
#include "base/AggrStat.h"
#include "base/ContTypeStat.h"

PtrArray<String*> ContTypeStat::TheKinds;

void ContTypeStat::RecordKind(int ctype, const String &kind) {
	Assert(ctype == TheKinds.count());
	Assert(kind);
	TheKinds.put(new String(kind), ctype);
}

void ContTypeStat::Store(OLog &ol) {
	OLogStorePtrs(ol, TheKinds);
}

void ContTypeStat::Load(ILog &il) {
	if (TheKinds.empty())
		ILogLoadPtrs(il, TheKinds, String);
	else {
		PtrArray<String*> kinds;
		ILogLoadPtrs(il, kinds, String);
		bool same(TheKinds.count() == kinds.count());
		for (int i = 0; same && i < kinds.count(); ++i)
			same = *kinds[i] == *TheKinds[i];

		if (!same) {
			cerr << "error: mismatching log files (logged content "
				"types differ):" << endl <<
				"content types in the first log:" << endl;
			for (int i = 0; i < TheKinds.count(); ++i)
				cerr << (i ? ", " : "\t") << *TheKinds[i];
			cerr << endl << "content types in the last log:" << endl;
			for (int i = 0; i < kinds.count(); ++i)
				cerr << (i ? ", " : "\t") << *kinds[i];
			cerr << endl;
			exit(-2);
		}
	}
}

ContTypeStat::ContTypeStat() {
}

ContTypeStat::~ContTypeStat() {
	while (theStats.count()) delete theStats.pop();
}

void ContTypeStat::record(int ctype, Size sz) {
	// initialize storage on-demand
	if (theStats.empty()) {
		theStats.stretch(TheKinds.count());
		for (int i = 0; i < TheKinds.count(); ++i)
			theStats.append(0);
	}
	Assert(0 <= ctype && ctype < theStats.count());
	if (!theStats[ctype])
		theStats[ctype] = new AggrStat;
	theStats[ctype]->record(sz);
}

void ContTypeStat::add(const ContTypeStat &s) {
	if (theStats.empty()) {
		theStats.stretch(s.theStats.count());
		for (int i = 0; i < s.theStats.count(); ++i) {
			AggrStat *const stat = s.theStats[i] ?
				new AggrStat(*s.theStats[i]) : 0;
			theStats.append(stat);
		}
	} else 
	if (!s.theStats.empty()) {
		Assert(theStats.count() == s.theStats.count());
		for (int i = 0; i < theStats.count(); ++i) {
			if (s.theStats[i]) {
				if (theStats[i])
					*theStats[i] += *s.theStats[i];
				else
					theStats[i] = new AggrStat(*s.theStats[i]);
			}
		}
	}
}

OLog &ContTypeStat::store(OLog &ol) const {
	OLogStorePtrs(ol, theStats);
	return ol;
}

ILog &ContTypeStat::load(ILog &il) {
	ILogLoadPtrs(il, theStats, AggrStat);
	return il;
}

ostream &ContTypeStat::print(ostream &os, const String &pfx) const {
	int count = 0;
	for (int c = 0; c < theStats.count(); ++c) {
		if (theStats[c] && theStats[c]->count())
			count++;
	}
	os << pfx << "categories:\t " << count << endl;
	for (int i = 0; i < theStats.count(); ++i) {
		if (theStats[i] && theStats[i]->count())
			theStats[i]->print(os, pfx + *TheKinds[i] + ".size.");
	}
	return os;
}
