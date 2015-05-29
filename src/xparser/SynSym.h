
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XPARSER_SYNSYM_H
#define POLYGRAPH__XPARSER_SYNSYM_H

#include "xparser/LexToken.h"

class SynSymTblItem;

// The SynSym class is the base class for all attributes
// that can be found on the generated parser's stack.
class SynSym {
	public:
		SynSym(const String &aType);
		virtual ~SynSym();

		const String &type() const { return theType; }
		virtual bool isA(const String &type) const;
		bool canBe(const String &type) const;
		bool equal(const SynSym &s) const;

		// aborts if incompatible types
		const SynSym &cast(const String &type) const;
		SynSym &cast(const String &type);

		// creates a copy of itself or compatible type
		// returns 0 if incompatible types and by default
		SynSym *clone(const String &type) const;
		SynSym *clone() const { return clone(type()); }

		// optional "nesting" support
		virtual SynSymTblItem **memberItem(const String &name);

		const TokenLoc &loc() const { return theLoc; }
		void loc(const TokenLoc &aLoc) { theLoc = aLoc; }

		virtual ostream &print(ostream &os, const String &pfx = 0) const;

	protected:
		void type(const String &aType);
		virtual SynSym *dupe(const String &) const { return 0; }

	protected:
		TokenLoc theLoc;

	private:
		String theType;
};

#define SymCast(Type, var) ((Type&)((var).cast(Type::TheType)))
#define ConstSymCast(Type, var) ((const Type&)((var).cast(Type::TheType)))

inline ostream &operator <<(ostream &os, const SynSym &sym) { return sym.print(os); }

#endif
