
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xparser/xparser.h"

#include "xparser/TokenSym.h"
#include "xparser/ParsSym.h"

const String ParsSym::TheType = "Pars";


ParsSym::ParsSym(const ParsSym &p): SynSym(TheType),
	theRuleName(p.ruleName()), theRhs(p.rhsCount()) {
	for (int i = 0; i < p.rhsCount(); ++i)
		append(p.rhs(i));
}

ParsSym::ParsSym(const String &aRuleName, int rhsCount):
	SynSym(TheType), theRuleName(aRuleName), theRhs(rhsCount) {
}

ParsSym::~ParsSym() {
	while (theRhs.count()) delete theRhs.pop();
}

bool ParsSym::isA(const String &type) const {
	return SynSym::isA(type) || type == TheType;
}

const SynSym &ParsSym::rhs(int idx) const {
	Assert(0 <= idx && idx < theRhs.count());
	Assert(theRhs[idx]);
	return *theRhs[idx];
}

const ParsSym &ParsSym::rhsRule(int idx) const {
	const SynSym &s = rhs(idx);
	return (const ParsSym &)s.cast(ParsSym::TheType);
}

const TokenSym &ParsSym::rhsToken(int idx) const {
	const SynSym &s = rhs(idx);
	return (const TokenSym &)s.cast(TokenSym::TheType);
}

SynSym *ParsSym::dupe(const String &aType) const {
	if (isA(aType))
		return new ParsSym(*this);
	else
		return SynSym::dupe(aType);
}

void ParsSym::append(const SynSym &s) {
	Should(!theRhs.full());
	theRhs.append(s.clone());
	Assert(theRhs.last());
}

ostream &ParsSym::print(ostream &os, const String &pfx) const {
	String p = pfx;
	bool tab = false;
	for (int i = 0; i < theRhs.count(); ++i) {
		if (rhs(i).isA(TokenSym::TheType)) {
			const TokenSym &token = rhsToken(i);
			if (tab)
				os << p;
			token.print(os, String());
			tab = false;

			if (token.spelling() == ";") {
				tab = true;
				os << endl;
			} else
			if (token.spelling() == "{") {
				tab = true;
				p += "\t";
				os << endl;
			} else
			if (token.spelling() == "}") {
				if (p.len())
					p = p(0, p.len()-1);
			}
		} else {
			rhs(i).print(os, p);
		}

	}
	return os;
}

