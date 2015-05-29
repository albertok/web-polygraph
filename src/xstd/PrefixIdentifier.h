
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_PREFIXIDENTIFIER_H
#define POLYGRAPH__XSTD_PREFIXIDENTIFIER_H

#include <ctype.h>
#include "xstd/String.h"
#include "xstd/TokenIdentifier.h"
#include "xstd/gadgets.h"


// PrefixIdentifier is a TokenIdentifier with an efficient prefix-based lookup.
//
// PrefixIdentifier look-up method checks if a given buffer starts
// with one of the "known" strings. If it does, an "id" of the
// corresponding string is returned.
// No "known" string can be prefix of the other -- we do not
// want to search for the longest prefix and such...
// A buffer is not expected to be zero-terminated.

// The search algorithm is very efficient. There is probably
// no optimal static (no caching, re-grouping) algorithm.

// note: the search tree does not de-allocate its parts on destruction

class PrefixIdentifier; // the user interface class, declared last

class PrefixIdTable;

// a node on the search tree (leaf or not)
class PrefixIdNode {
	public:
		PrefixIdNode();

		bool dead() const { return theId == 0; }
		bool leaf() const { return theId > 0; }
		int id() const { return theId; }

		int lookup(const char *buf, int len) const;
		void add(String *str, int id, int myLevel);

		void optimize();

		void report(ostream &os, int myLevel) const;

	protected:
		union {
			PrefixIdTable *theTab; // lookup table (intermediate nodes)
			String *theStr;        // known-string (leaf nodes)
		} u;
		int theId;              // id > 0 => leaf, id < 0 => intermed
};


// collection of nodes indexed by a char at a given search pos
class PrefixIdTable {
	public:
		PrefixIdTable(int aPos);

		int lookup(const char *buf, int len) const;
		void add(String *str, int id);

		void optimize();
		bool single(PrefixIdNode &n);

		void report(ostream &os, int myLevel) const;

	protected:
		inline int lookupIdx(const String &str) const;
		// returns -1 if len <= theLookupPos
		inline int lookupIdx(const char *buf, int len) const;
		
	protected:
		Array<PrefixIdNode> theNodes;
		int theLookupPos;
};


// user interface: build(add everything), optimize, look-up.
class PrefixIdentifier: public TokenIdentifier {
	public:
		PrefixIdentifier(): isLocked(false) {}

		virtual int lookup(const char *buf, int len) const;
		virtual int lookup(const String &str) const;

		// call this after all additions
		void optimize();

		// prints a search tree
		void report(ostream &os) const;

	protected:
		virtual void doAdd(String &str, int id);

	protected:
		PrefixIdNode theHead;

		bool isLocked; // additions are fatal, set in optimize()
};


/* inlined methods */

inline 
int PrefixIdTable::lookupIdx(const String &str) const {
	return str.len() > theLookupPos ?
		xord(toupper(str[theLookupPos])) : theLookupPos;
}

inline
int PrefixIdTable::lookupIdx(const char *buf, int len) const {
	return len > theLookupPos ?
		xord(toupper(buf[theLookupPos])) : -1;
}

#endif
