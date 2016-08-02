
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_CONTTYPESTAT_H
#define POLYGRAPH__BASE_CONTTYPESTAT_H

#include "xstd/Array.h"
#include "base/LogObj.h"
#include "base/TmSzHistStat.h"

class String;

// statistics collection for built-in and PGL-configured Content types
namespace ContType {

// record a built-in or PGL-configured content type with the given index
void Record(const int ctype, const String &kind);
int Count(); // number of recorded content types
// recorded content type with the given index
const String &Kind(const int idx);
// note that all built-in content types are recorded
void NoteNormalContentStart();
// first PGL-configured Content index, built-in content types have lower indexes
int NormalContentStart();

void Store(OLog &ol); // store recorded content types to binary log
void Load(ILog &il); // load recorded content type from binary log

// groups StatT stats for all content types
template <class StatT>
class Stat: public LogObj {
	public:
		Stat() {}
		Stat(const Stat &s) { add(s); }
		virtual ~Stat();

		void reset();

		bool sane() const;
		const StatT *hasStats(const int idx) const;
		const StatT &stats(const int idx) const { return *theStats[idx]; }

		void record(const int ctype, const Time &tm, const Size sz);
		void add(const Stat &s);

		virtual OLog &store(OLog &ol) const;
		virtual ILog &load(ILog &il);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		Array<StatT*> theStats;
};

typedef Stat<TmSzStat> AggrStat;
typedef Stat<TmSzHistStat> HistStat;


template <class StatT>
Stat<StatT>::~Stat() {
	while (theStats.count()) delete theStats.pop();
}

template <class StatT>
void Stat<StatT>::reset() {
	for (int i = 0; i < theStats.count(); ++i) {
		if (theStats[i])
			theStats[i]->reset();
	}
}

template <class StatT>
bool Stat<StatT>::sane() const {
	bool isSane = true;
	for (int i = 0; isSane && i < theStats.count(); ++i) {
		if (theStats[i])
			isSane = theStats[i]->sane();
	}
	return isSane;
}

template <class StatT>
const StatT *Stat<StatT>::hasStats(const int idx) const {
	return (0 <= idx && idx < theStats.count()) ? theStats[idx] : 0;
}

template <class StatT>
void Stat<StatT>::record(const int ctype, const Time &tm, const Size sz) {
	// initialize storage on-demand
	if (theStats.empty()) {
		theStats.stretch(Count());
		for (int i = 0; i < Count(); ++i)
			theStats.append(0);
	}
	Assert(0 <= ctype && ctype < theStats.count());
	if (!theStats[ctype])
		theStats[ctype] = new StatT;
	theStats[ctype]->record(tm, sz);
}

template <class StatT>
void Stat<StatT>::add(const Stat &s) {
	if (theStats.empty()) {
		theStats.stretch(s.theStats.count());
		for (int i = 0; i < s.theStats.count(); ++i) {
			StatT *const stat = s.theStats[i] ?
				new StatT(*s.theStats[i]) : 0;
			theStats.append(stat);
		}
	} else if (!s.theStats.empty()) {
		Assert(theStats.count() == s.theStats.count());
		for (int i = 0; i < theStats.count(); ++i) {
			if (s.theStats[i]) {
				if (theStats[i])
					*theStats[i] += *s.theStats[i];
				else
					theStats[i] = new StatT(*s.theStats[i]);
			}
		}
	}
}

template <class StatT>
OLog &Stat<StatT>::store(OLog &ol) const {
	return ol << theStats;
}

template <class StatT>
ILog &Stat<StatT>::load(ILog &il) {
	return il >> theStats;
}

template <class StatT>
ostream &Stat<StatT>::print(ostream &os, const String &pfx) const {
	int count = 0;
	for (int c = 0; c < theStats.count(); ++c) {
		if (theStats[c] && theStats[c]->known())
			count++;
	}
	os << pfx << "categories:\t " << count << endl;
	Must(theStats.count() <= Count());
	for (int i = 0; i < theStats.count(); ++i) {
		if (theStats[i] && theStats[i]->known())
			theStats[i]->print(os, pfx + Kind(i) + '.');
	}
	return os;
}

}

#endif
