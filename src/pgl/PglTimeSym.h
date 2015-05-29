
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLTIMESYM_H
#define POLYGRAPH__PGL_PGLTIMESYM_H

#include "xstd/Time.h"
#include "pgl/PglExprSym.h"

// time

class TimeSym: public ExpressionSym {
	public:
		static String TheType;

	public:
		TimeSym(Time aVal);

		Time val() const { return theVal; }
		void val(Time v) { theVal = v; }

		virtual bool isA(const String &type) const;

		virtual ExpressionSym *unOper(const Oper &op) const;
		virtual ExpressionSym *bnOper(const Oper &op, const SynSym &exp) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		ExpressionSym *operTT(const Oper &op, const SynSym &exp) const;
		ExpressionSym *operTN(const Oper &op, const SynSym &exp) const;

	protected:
		Time theVal;
};

#endif
