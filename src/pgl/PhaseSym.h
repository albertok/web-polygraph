
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PHASESYM_H
#define POLYGRAPH__PGL_PHASESYM_H

#include "xstd/String.h"
#include "xstd/Array.h"
#include "pgl/PglRecSym.h"

class GoalSym;
class RptmstatSym;
class CodeSym;
class StatsSampleSym;

// base class for robot and server symbols
class PhaseSym: public RecSym {
	public:
		typedef bool (PhaseSym::*FactorFunc)(double &rf) const;

	public:
		static const String TheType;

	public:
		PhaseSym();
		PhaseSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		void name(const String &aName);

		String name() const;
		GoalSym *goal() const;
		RptmstatSym *rptmstat() const;
		CodeSym *script() const;
		bool populusFactorBeg(double &f) const;
		bool populusFactorEnd(double &f) const;
		bool loadFactorBeg(double &f) const;
		bool loadFactorEnd(double &f) const;
		bool recurFactorBeg(double &rf) const;
		bool recurFactorEnd(double &rf) const;
		bool specialMsgFactorBeg(double &f) const;
		bool specialMsgFactorEnd(double &f) const;
		bool statsSamples(Array<const StatsSampleSym*> &samples) const;
		bool logStats(bool &ls) const;
		bool primary(bool &ls) const;
		bool waitWssFreeze(bool &wwf) const;
		bool synchronize(bool &s) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
