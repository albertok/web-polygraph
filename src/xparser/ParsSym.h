
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XPARSER_PARSSYM_H
#define POLYGRAPH__XPARSER_PARSSYM_H

#include "xstd/Array.h"
#include "xparser/SynSym.h"

class TokenSym;

// represents a result of parsing a syntax rule
// "name :- token" rules are usually represented by a more specific class
class ParsSym: public SynSym {
	public:
		static const String TheType;

	public:
		ParsSym(const ParsSym &p);
		ParsSym(const String &aRuleName, int rhsCount);
		virtual ~ParsSym();

		virtual bool isA(const String &type) const;

		const String &ruleName() const { return theRuleName; }
		int rhsCount() const { return theRhs.count(); }

		const SynSym &rhs(int idx) const;
		const ParsSym &rhsRule(int idx) const;
		const TokenSym &rhsToken(int idx) const;

		void append(const SynSym &s);

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &) const;

	private:
		String theRuleName;
		Array<SynSym*> theRhs;
};

#endif
