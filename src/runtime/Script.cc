
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"
#include "base/StatIntvlRec.h"
#include "base/polyLogCats.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglListSym.h"
#include "pgl/PglCodeSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/GoalSym.h"
#include "pgl/StatSampleSym.h"
#include "runtime/ErrorMgr.h"
#include "runtime/StatIntvl.h"
#include "runtime/DutWatchdog.h"
#include "runtime/LogComment.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/StatPhase.h"
#include "runtime/Script.h"
#include "runtime/polyErrors.h"


Script::Script(const SynSym &aCode): theCode(aCode.clone()), theSampler(0) {
}

Script::~Script() {
	delete theCode;
	theCode = 0;
}

void Script::statsSampler(Sampler *aSampler) {
	Should(!aSampler ^ !theSampler);
	theSampler = aSampler;
}

const SynSym &Script::code() const {
	Assert(theCode);
	return *theCode;
}

void Script::run() {
	if (!DutWatchdog::IsEnabled)
		return;

	if (Should(theCode)) {
		openContext(theCode->loc().fname()); // add lineno
		interpret(*theCode);
		closeContext();
	}
}

ExpressionSym *Script::callFunc(const String &cname, const ListSym &args) {
	if (cname == "currentLoadFactor") {
		checkArgs(cname, 0, args);
		const double factor = TheStatPhaseMgr->loadFactor().current();
		return new NumSym(factor); // Place
	}

	if (cname == "currentPopulusFactor") {
		checkArgs(cname, 0, args);
		const double factor = TheStatPhaseMgr->populusFactor().current();
		return new NumSym(factor);
	}

	if (cname == "currentSample" && Should(theSampler)) {
		checkArgs(cname, 0, args);
		StatIntvl::UpdateLiveStats();
		const StatIntvlRec &s = theSampler->getRec(lgcCltSide);
		return new StatSampleSym(s);
	}

	if (cname == "currentPhase") {
		checkArgs(cname, 0, args);
		StatIntvl::UpdateLiveStats();
		const StatIntvlRec &s = TheStatPhaseMgr->getRec(lgcCltSide);
		return new StatSampleSym(s);
	}

	return PglSemx::callFunc(cname, args);
}

void Script::callProc(const String &cname, const ListSym &args) {
	if (cname == "changeLoadFactorBy") {
		checkArgs(cname, 1, args);
		const NumSym &ratios = (const NumSym&)
			extractArg(cname, 0, args, NumSym::TheType);
		const double ratio = ratios.val();
		TheStatPhaseMgr->loadFactor().changeBy(ratio);
		return;
	}

	if (cname == "setLoadFactorTo") {
		checkArgs(cname, 1, args);
		const NumSym &ratios = (const NumSym&)
			extractArg(cname, 0, args, NumSym::TheType);
		const double ratio = ratios.val();
		TheStatPhaseMgr->loadFactor().setTo(ratio);
		return;
	}

	if (cname == "changePopulusFactorBy") {
		checkArgs(cname, 1, args);
		const NumSym &ratios = (const NumSym&)
			extractArg(cname, 0, args, NumSym::TheType);
		const double ratio = ratios.val();
		TheStatPhaseMgr->populusFactor().changeBy(ratio);
		return;
	}

	if (cname == "setPopulusFactorTo") {
		checkArgs(cname, 1, args);
		const NumSym &ratios = (const NumSym&)
			extractArg(cname, 0, args, NumSym::TheType);
		const double ratio = ratios.val();
		TheStatPhaseMgr->populusFactor().setTo(ratio);
		return;
	}

	if (cname == "setSamplingGoalTo") {
		checkArgs(cname, 1, args);
		const GoalSym &goal = (const GoalSym&)
			extractArg(cname, 0, args, GoalSym::TheType);
		theSampler->configureGoal(goal);
		return;
	}

	if (cname == "print") {
		// XXX: would not be needed when we start capturing
		// cout into binary log
		print(Comment(5), args) << endc;
		return;
	}

	if (cname == "printV") {
		if (args.count() >= 1) {
			const int verbLevel = anyToInt(*args[0]);
			print(Comment(verbLevel), args, 1) << endc;
		} else if (ReportError(errPglScript)) {
			Comment << args.loc() << "bad argument count for '" <<
				cname << "': expected at least 1 argument, got "
				<< args.count() << endc;
		}
		return;
	}

	if (cname == "reachedPositiveGoal") {
		if (args.count() > 1) {
			cerr << args.loc() << "bad argument count for '" <<
				cname << "': expected at most 1 argument, got "
				<< args.count() << endl << xexit;
		}
		String reason;
		if (args.count() == 1) {
			const StringSym &sym = (const StringSym&)
				extractArg(cname, 0, args, StringSym::TheType);
			reason = sym.val();
		}
		TheStatPhaseMgr->reachedPositiveGoal(reason);
		return;
	}

	PglSemx::callProc(cname, args);
}

void Script::execEveryCode(const EveryCodeSym &ecode) {
	DutWatchdog *dog = new DutWatchdog;
	dog->configure(ecode);
	TheStatPhaseMgr->addWatchdog(dog);
	dog->start();
}

int Script::logCat() const {
	return lgcCltSide;
}
