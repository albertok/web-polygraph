
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLDISTRSYM_H
#define POLYGRAPH__PGL_PGLDISTRSYM_H

#include "pgl/PglExprSym.h"

class String;
class RndDistr;
class QualifSym;

// a distribution with an optional qualifier

class DistrSym: public ExpressionSym {
	public:
		static void IntArgPrinter(ostream &os, double arg, int);
		static void SizeArgPrinter(ostream &os, double arg, int);
		static void TimeArgPrinter(ostream &os, double arg, int);
		static String TheType;

	public:
		DistrSym(const String &aType, RndDistr *aVal);
		virtual ~DistrSym();

		virtual bool isA(const String &type) const;

		RndDistr *val() const { return theVal; }

		void qualifier(const QualifSym *aQual, const String &anOp);
		QualifSym *qualifier() const { return theQual; }
		const String &qfOp() const { return theQfOp; }

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

	protected:
		mutable RndDistr *theVal;
		QualifSym *theQual; // qualifier
		String theQfOp;     // qualifier operator
};

#endif
