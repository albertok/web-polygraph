
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_REGEXGROUPS_H
#define POLYGRAPH__CLIENT_REGEXGROUPS_H

#include "xstd/Array.h"
#include "client/RegExGroup.h"

class RegEx;

// an RE group consisting of one RE with no augmenting operations
class RegExGrpOne: public RegExGroup {
	public:
		RegExGrpOne(RegEx *aVal);

		virtual bool match(const RegExMatchee &m, int flags = 0, int *errNo = 0) const;
		virtual ostream &print(ostream &os) const;

	protected:
		bool matchUrl(const RegExMatchee &m, int flags, int *errNo) const;
		bool matchGroups(const RegExMatchee &m, int flags, int *errNo) const;
		const char *hostIps(const char *hostName) const;

	protected:
		RegEx *theVal;
};

// a !RE group
class RegExGrpNot: public RegExGroup {
	public:
		RegExGrpNot(RegExGroup *aVal);

		virtual bool match(const RegExMatchee &m, int flags = 0, int *errNo = 0) const;
		virtual ostream &print(ostream &os) const;

	protected:
		RegExGroup *theVal;
};

// an Operator(re1, re2, ...) group
class RegExGrpOper: public RegExGroup {
	public:
		void add(RegExGroup *item);

		//virtual bool match(const RegExMatchee &m, int flags = 0, int *errNo = 0) const;
		virtual ostream &print(ostream &os) const;

	protected:
		virtual const char *operImage() const = 0; // for print()ing

	protected:
		Array<RegExGroup*> theItems;
};

// an And(re1, re2, ...) group
class RegExGrpAnd: public RegExGrpOper {
	public:
		virtual bool match(const RegExMatchee &m, int flags = 0, int *errNo = 0) const;

	protected:
		virtual const char *operImage() const;
};

// an Or(re1, re2, ...) group
class RegExGrpOr: public RegExGrpOper {
	public:
		virtual bool match(const RegExMatchee &m, int flags = 0, int *errNo = 0) const;

	protected:
		virtual const char *operImage() const;
};

// these may seem to simple and boring, 
// but that is because we have not added optimizations yet

#endif
