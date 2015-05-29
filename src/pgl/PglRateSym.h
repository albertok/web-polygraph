
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLRATESYM_H
#define POLYGRAPH__PGL_PGLRATESYM_H

#include "xstd/Time.h"
#include "pgl/PglExprSym.h"

class TimeSym;
class SizeSym;
class NumSym;

// rate [objects/time]

class RateSym: public ExpressionSym {
	public:
		static String TheType;

	public:
		RateSym(double aCount, Time AnInterval);
		RateSym(double aVal);

		double val() const; // [obj/sec]
		void val(double aVal); // [obj/sec]
		//void val(double aCount, Time AnInterval);

		virtual bool isA(const String &type) const;

		virtual ExpressionSym *unOper(const Oper &op) const;
		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		ExpressionSym *operRR(const Oper &op, RateSym *rs, const SynSym &s) const;
		ExpressionSym *operRT(const Oper &op, TimeSym *ts, const SynSym &s) const;
		ExpressionSym *operRS(const Oper &op, SizeSym *ss, const SynSym &s) const;
		ExpressionSym *operRN(const Oper &op, NumSym *ns, const SynSym &s) const;

	protected:
		double theCount;
		Time theInterval;
};

#endif
