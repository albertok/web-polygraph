
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/String.h"
#include "xstd/TokenIdentifier.h"


/* TokenIdentifier */

TokenIdentifierIter TokenIdentifier::iterator() const {
	return TokenIdentifierIter(*this);
}

int TokenIdentifier::add(const String &str) {
	return add(str, theLastId + 1);
}

int TokenIdentifier::add(const String &str, int id) {
	Assert(theLastId != id); // weak check for uniqueness
	theCount++;
	String *const copy = new String(str);
	theStrings.put(copy, id);
	doAdd(*copy, id);
	return theLastId = id;
}


/* TokenIdentifierIter */

TokenIdentifierIter::TokenIdentifierIter(const TokenIdentifier &anIdfr):
	theIdfr(anIdfr), thePos(0) {
	sync();
}

TokenIdentifierIter &TokenIdentifierIter::operator ++() {
	if (!atEnd()) {
		++thePos;
		sync();
	}
	return *this;
}

void TokenIdentifierIter::sync() {
	while (thePos < theIdfr.theStrings.count() && !theIdfr.theStrings[thePos]) {
		++thePos;
	}
}

bool TokenIdentifierIter::atEnd() const {
	return thePos >= theIdfr.theStrings.count();
}

const String &TokenIdentifierIter::str() const {
	Assert(!atEnd());
	return *theIdfr.theStrings[thePos];
}

int TokenIdentifierIter::id() const {
	Assert(!atEnd());
	return thePos; // same as id in this StrIdentifier implementation
}
