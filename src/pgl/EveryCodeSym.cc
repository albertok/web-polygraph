
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "pgl/pgl.h"

#include "pgl/PglCodeSym.h"
#include "pgl/GoalSym.h"
#include "pgl/EveryCodeSym.h"


String EveryCodeSym::TheType = "EveryCode";


EveryCodeSym::EveryCodeSym(): ExpressionSym(EveryCodeSym::TheType),
	theGoal(0), theCode(0) {
}

EveryCodeSym::EveryCodeSym(const GoalSym &aGoal, const CodeSym &aCode):
	ExpressionSym(EveryCodeSym::TheType), theGoal(0), theCode(0) {
	theGoal = (GoalSym*)aGoal.clone();
	theCode = (CodeSym*)aCode.clone();
}

EveryCodeSym::EveryCodeSym(GoalSym *aGoal, CodeSym *aCode):
	ExpressionSym(EveryCodeSym::TheType), theGoal(0), theCode(0) {
	theGoal = aGoal;
	theCode = aCode;
}

EveryCodeSym::~EveryCodeSym() {
	delete theGoal;
	delete theCode;
}

bool EveryCodeSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *EveryCodeSym::dupe(const String &type) const {
	if (isA(type)) {
		if (goal() && call())
			return new EveryCodeSym(*goal(), *call());
		Should(!goal() && !call());
		return new EveryCodeSym();
	} else {
		return ExpressionSym::dupe(type);
	}
}

const GoalSym *EveryCodeSym::goal() const {
	return theGoal;
}

const CodeSym *EveryCodeSym::call() const {
	return theCode;
}

ostream &EveryCodeSym::print(ostream &os, const String &pfx) const {
	if (goal() && call()) {
		os << pfx << "every ";
		goal()->print(os, pfx + "\t");
		os << pfx << " do ";
		call()->print(os, pfx + "\t");
	} else {
		Should(!goal() && !call());
		os << pfx << "undef()";
	}
	return os;
}
