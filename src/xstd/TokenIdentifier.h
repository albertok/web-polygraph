
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_TOKENIDENTIFIER_H
#define POLYGRAPH__XSTD_TOKENIDENTIFIER_H

#include "xstd/Array.h"

// A String to unique integer ID mapping. Kids determine whether a
// prefix or a full string match is needed and how efficient the lookup
// is. See PrefixIdentifier and StringIdentifier classes.

class String;

class TokenIdentifier; // the user interface class, declared last

// iterator for TokenIdentifier
class TokenIdentifierIter {
	public:
		TokenIdentifierIter(const TokenIdentifier &anIdfr);

		operator void*() const { return atEnd() ? (void*)0 : (void*)-1; }
		bool atEnd() const;
		TokenIdentifierIter &operator ++();

		const String &str() const;
		int id() const;

	protected:
		void sync();

	protected:
		const TokenIdentifier &theIdfr;
		int thePos;
};


class TokenIdentifier {
	public:
		friend class TokenIdentifierIter;
		typedef TokenIdentifierIter Iter;

	public:
		TokenIdentifier(): theCount(0), theLastId(0) {}
		virtual ~TokenIdentifier() {}

		int count() const { return theCount; }
		const String &string(int id) const { return *theStrings[id]; }
		virtual int lookup(const char *buf, int len) const = 0;
		virtual int lookup(const String &str) const = 0;

		Iter iterator() const;

		// to be safe, use one or the other of a given class:
		int add(const String &str);          // next available id
		int add(const String &str, int id);  // small user-specified id

	protected:
		virtual void doAdd(String &str, int id) = 0;

		PtrArray<String *> theStrings; // reverse index (id -> str)
		int theCount;
		int theLastId;
};

#endif
