
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <stdlib.h>

#include "base/ErrorRec.h"
#include "base/ILog.h"
#include "base/OLog.h"
#include "base/ErrorHash.h"


ErrorHash::ErrorHash(int aCapacity): theCount(0) {
	theHash.resize(aCapacity);
}

ErrorHash::~ErrorHash() {
	for (int i = 0; i < theHash.count(); ++i) {
		ErrorRec *r = theHash[i];
		while (r) {
			ErrorRec *next = r->next();
			delete r;
			r= next;
		}
	}
}

const ErrorRec *ErrorHash::find(const Error &e) const {
	return *findPos(e);
}

ErrorRec *ErrorHash::findOrAdd(const Error &e) {
	ErrorRec **pos = findPos(e);
	if (!*pos) {
		*pos = new ErrorRec(e);
		theCount++;
	}
	return *pos;
}

void ErrorHash::add(const ErrorHash &h) {
	for (ErrorHashIter i = h.iterator(); i; ++i) {
		ErrorRec **pos = findPos(*i);
		if (!*pos) {
			*pos = new ErrorRec(*i);
			theCount++;
		} else {
			(*pos)->add(*i);
		}
	}
}

void ErrorHash::store(OLog &ol) const {
	ol << theCount;
	for (ErrorHashIter i = iterator(); i; ++i) {
		i->store(ol);
	}
}

void ErrorHash::load(ILog &il) {
	Assert(!theCount);
	il >> theCount;
	for (int i = 0; i < theCount; ++i) {
		ErrorRec *r = new ErrorRec;
		r->load(il);
		ErrorRec **pos = findPos(*r);
		Assert(!*pos);
		*pos = r;
	}
}

ErrorHashIter ErrorHash::iterator() const {
	return ErrorHashIter(this);
}

int ErrorHash::hash(const Error &e) const {
	return abs(e.no()) % theHash.capacity();
}

ErrorRec **ErrorHash::findPos(const Error &e) {
	ErrorRec **pos = &theHash[hash(e)];
	while (*pos && (*pos)->no() != e.no())
		pos = &(*pos)->next();
	return pos;
}

ErrorRec *const *ErrorHash::findPos(const Error &e) const {
	ErrorRec *const *pos = &theHash[hash(e)];
	while (*pos && (*pos)->no() != e.no())
		pos = &(*pos)->next();
	return pos;
}


/* ErrorHashIter */

ErrorHashIter::ErrorHashIter(const ErrorHash *aHash):
	theHash(aHash), theRec(0), theBucket(-1) {

	Assert(theHash);
	next();
}

void ErrorHashIter::next() {
	// move within a bucket
	if (theRec)
		theRec = theRec->next();

	// find next non-empty bucket
	while (!theRec && ++theBucket < theHash->theHash.count())
		theRec = theHash->theHash[theBucket];
}
