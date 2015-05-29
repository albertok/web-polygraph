
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSIZESYM_H
#define POLYGRAPH__PGL_PGLSIZESYM_H

#include "xstd/BigSize.h"
#include "pgl/PglExprSym.h"

class TimeSym;
class BwidthSym;
class RateSym;
class NumSym;

// size

class SizeSym: public ExpressionSym {
	public:
		static String TheType;

	public:
		SizeSym(const BigSize &aVal);

		BigSize val() const { return theVal; }
		void val(BigSize v) { theVal = v; }

		virtual bool isA(const String &type) const;

		virtual ExpressionSym *unOper(const Oper &op) const;
		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		ExpressionSym *operSS(const Oper &op, SizeSym *ss, const SynSym &exp) const;
		ExpressionSym *operST(const Oper &op, TimeSym *ts, const SynSym &s) const;
		ExpressionSym *operSR(const Oper &op, RateSym *rs, const SynSym &s) const;
		ExpressionSym *operSB(const Oper &op, BwidthSym *bs, const SynSym &s) const;
		ExpressionSym *operSN(const Oper &op, NumSym *ns, const SynSym &exp) const;

	protected:
		BigSize theVal;
};

#endif
