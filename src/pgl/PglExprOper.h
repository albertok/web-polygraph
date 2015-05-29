
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLEXPROPER_H
#define POLYGRAPH__PGL_PGLEXPROPER_H

class TokenSym;

// an expression operator 
// safer than using operator (symbol) IDs directly
// more efficient than using operator (symbol) spelling
class ExpressionOper {
	public:
		static ExpressionOper ToInt();
		static ExpressionOper Same();

	public:
		ExpressionOper(const TokenSym &t);

		const String &image() const;


		bool boolNot() const;
		bool boolAnd() const;
		bool boolOr() const;
		bool boolXor() const;
		bool boolBin() const { return boolAnd() || boolOr() || boolXor(); }
		bool boolAny() const { return boolBin() || boolNot(); }

		bool same() const;
		bool diff() const;
		bool lessOrEq() const;
		bool lessTrue() const;
		bool greaterOrEq() const;
		bool greaterTrue() const;
		bool equality() const { return same() || diff(); }
		bool comparison() const { return equality() ||
			lessOrEq() || lessTrue() || greaterOrEq() || greaterTrue(); }

		bool plus() const;
		bool minus() const;

		bool mult() const;
		bool div() const;
		bool mod() const;

		bool power() const;
		bool range() const;
		bool clone() const;

		bool arithmAny() const { return plus() || minus() || mult() || div() || mod() || power(); }

	private:
		String theImage;
		int theId;
};

#endif
