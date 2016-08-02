
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Error.h"
#include "xstd/gadgets.h"
#include "base/ILog.h"
#include "base/OLog.h"
#include "base/ErrorRec.h"
#include "base/ErrorHash.h"
#include "base/ErrorStat.h"


ErrorStat::ErrorStat(): theHash(0), theCount(0) {
	newHash();
}

ErrorStat::~ErrorStat() {
	delete theHash;
}

void ErrorStat::reset() {
	delete theHash;
	newHash();
	theCount = 0;
}

void ErrorStat::newHash() {
	theHash = new ErrorHash(251);
}

bool ErrorStat::record(const Error &e, ErrorRec *&rec, const Counter count) {
	Assert(count);

	rec = theHash->findOrAdd(e);
	Assert(rec);

	theCount += count;
	rec->noteError(count);
	return rec->needReport();
}

void ErrorStat::add(const ErrorStat &s) {
	theCount += s.theCount;
	theHash->add(*s.theHash);
}

ErrorStat &ErrorStat::operator =(const ErrorStat &es) {
	reset();
	add(es);
	return *this;
}

Counter ErrorStat::count(const Error &e) const {
	if (const ErrorRec *rec = theHash->find(e))
		return rec->count();
	else
		return 0;
}

OLog &ErrorStat::store(OLog &ol) const {
	theHash->store(ol);
	return ol << theCount;
}

ILog &ErrorStat::load(ILog &il) {
	reset();
	theHash->load(il);
	return il >> theCount;
}

static
int cmpErrRecPtr(const void *p1, const void *p2) {
	if (const int cmp = (*(const ErrorRec**)p1)->count() - (*(const ErrorRec**)p2)->count())
		return -cmp;
	else
		return (*(const ErrorRec**)p1)->no() - (*(const ErrorRec**)p2)->no();
}

int ErrorStat::index(Index &idx) const {
	idx.stretch(theHash->count());
	for (ErrorHashIter i = theHash->iterator(); i; ++i)
		idx.append(&*i);
	qsort(idx.items(), idx.count(), sizeof(ErrorRec*), &cmpErrRecPtr);
	return idx.count();
}

ostream &ErrorStat::print(ostream &os, const String &pfx) const {
	os << pfx << "count:\t " << count() << endl;

	Index prnIdx;
	if (index(prnIdx)) {
		os << pfx << "tbl:" << endl;
		os << "#count    count% explanation" << endl;

		for (int j = 0; j < prnIdx.count(); ++j) {
			const ErrorRec *rec = prnIdx[j];
			Assert(rec);
			os << setw(9) << rec->count()
				<< ' ' << setw(6) << Percent(rec->count(), count())
				<< ' ' << '"';
			rec->print(os) << '"' << endl;
		}

		os << endl;
	}

	return os;
}
