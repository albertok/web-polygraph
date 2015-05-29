
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_ERRORHASH_H
#define POLYGRAPH__BASE_ERRORHASH_H

#include "xstd/Array.h"

class Error;
class ErrorRec;
class ErrorHash;

class ErrorHashIter {
	public:
		ErrorHashIter(const ErrorHash *aHash);

		operator void *() const { return theRec ? (void*)-1 : 0; }

		const ErrorRec &operator *() const { return *theRec; }
		const ErrorRec *operator ->() const { return theRec; }

		ErrorHashIter &operator ++() { next(); return *this; }

	protected:
		void next();

	protected:
		const ErrorHash *theHash;
		ErrorRec *theRec;
		int theBucket;
};

class ErrorHash {
	friend class ErrorHashIter;

	public:
		ErrorHash(int aCapacity);
		~ErrorHash();

		int count() const { return theCount; }

		const ErrorRec *find(const Error &e) const;
		ErrorRec *findOrAdd(const Error &e);

		void add(const ErrorHash &h);

		void store(OLog &ol) const;
		void load(ILog &il);

		ErrorHashIter iterator() const;

	protected:
		int hash(const Error &e) const;
		ErrorRec **findPos(const Error &e);
		ErrorRec *const *findPos(const Error &e) const;

	protected:
		Array<ErrorRec*> theHash;
		int theCount;             // unique errors hashed
};

#endif
