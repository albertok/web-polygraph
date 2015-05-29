
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_EVERYCODESYM_H
#define POLYGRAPH__PGL_EVERYCODESYM_H

#include "pgl/PglExprSym.h"

class GoalSym;
class CodeSym;

// conditional code; used by phase watchdogs
class EveryCodeSym: public ExpressionSym {
	public:
		static String TheType;

	public:
		EveryCodeSym();
		EveryCodeSym(const GoalSym &aGoal, const CodeSym &aCode);
		EveryCodeSym(GoalSym *aGoal, CodeSym *aCode);
		virtual ~EveryCodeSym();

		const GoalSym *goal() const;
		const CodeSym *call() const;

		virtual bool isA(const String &type) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

	protected:
		GoalSym *theGoal;
		CodeSym *theCode;
};

#endif
