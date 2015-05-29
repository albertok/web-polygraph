
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XPARSER_TOKENSYM_H
#define POLYGRAPH__XPARSER_TOKENSYM_H

#include "xparser/SynSym.h"

// The TokenSym class maintains "token" attributes that
// store tokens as they come from a Lexer

class TokenSym : public SynSym {
	public:
		static const String TheType;

	public:
		TokenSym(const String &n, int anId = -1);

		int id() const { return theId; }
		const String &spelling() const { return theSpell; }

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &) const;

	private:
		String theSpell;
		int theId;
};

#endif
