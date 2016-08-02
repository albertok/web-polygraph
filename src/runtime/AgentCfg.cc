
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "xstd/Rnd.h"
#include "xstd/rndDistrs.h"
#include "xstd/StringIdentifier.h"
#include "xstd/h/iomanip.h"
#include "base/RndPermut.h"
#include "pgl/AgentSym.h"
#include "pgl/PglStringSym.h"
#include "runtime/SslWraps.h"
#include "runtime/Agent.h"
#include "runtime/AgentCfg.h"
#include "runtime/LogComment.h"
#include "runtime/MimeHeadersCfg.h"
#include "runtime/XactAbortCoord.h"


AgentCfg::AgentCfg(): theSslWrapSel(0), theHttpVersionSel(0),
	theHttpHeaders(0),
	theCookieSenderProb(0), theAbortProb(0) {
}

AgentCfg::~AgentCfg() {
	delete theSslWrapSel;
	delete theHttpVersionSel;
	delete theHttpHeaders;
}

void AgentCfg::configure(const AgentSym *agent) {
	Assert(agent);

	configureSslWraps(agent);
	configureCustomStatsScope(agent);
	configureHttpVersions(agent);
	configureHttpHeaders(agent);

	agent->abortProb(theAbortProb);
}

void AgentCfg::configureSslWraps(const AgentSym *agent) {
	Array<SslWrapSym*> syms;
	if (!agent->sslWraps(syms, theSslWrapSel))
		return; // no SSL wrappers configured

	TheSslWraps.get(syms, theSslWraps);
	theSslWrapSel->rndGen(GlbRndGen("ssl_wrappers"));
}

bool AgentCfg::selectSslWrap(const SslWrap *&wrap) {
	if (!theSslWraps.count())
		return false;

	const int wrapIdx = (int)theSslWrapSel->trial();
	if (!Should(0 <= wrapIdx && wrapIdx < theSslWraps.count()))
		return false;

	wrap = theSslWraps[wrapIdx];
	return true;
}

bool AgentCfg::selectCookieSenderStatus() {
	static RndGen rng;
	return rng.event(theCookieSenderProb);
}

void AgentCfg::selectAbortCoord(XactAbortCoord &coord) {
	static RndGen rng1, rng2; // uncorrelated unless theAbortProb is 1
	if (rng1.event(theAbortProb)) {
		const RndGen::Seed where = rng2.state();
		rng2.trial();
		coord.configure(rng2.state(), where);
	} else {
		const RndGen::Seed whether = rng1.state();
		rng1.trial();
		coord.configure(whether, rng1.state());
	}
}

void AgentCfg::configureHttpVersions(const AgentSym *agent) {
	static StringIdentifier sidf;
	if (!sidf.count()) {
		sidf.add("1.0", Agent::protoHttp1p0);
		sidf.add("1.1", Agent::protoHttp1p1);
	}

	theHttpVersionSel = agent->httpVersions(sidf);
	if (!theHttpVersionSel)
		theHttpVersionSel = new ConstDistr(0, Agent::protoHttp1p1); // default
	theHttpVersionSel->rndGen(LclRndGen("http_versions"));
}

int AgentCfg::selectHttpVersion() {
	return (int)theHttpVersionSel->trial();
}

void AgentCfg::configureHttpHeaders(const AgentSym *agent) {
	if (const ArraySym *const a = agent->httpHeaders())
		theHttpHeaders = new MimeHeadersCfg(*a);
}

bool AgentCfg::inCustomStatsScope(const int httpStatus) const {
	return 0 <= httpStatus && httpStatus < NumberOfHttpStatusCodes &&
		theCustomStatsScope[httpStatus];
}

// Parses strings of "[+-]NNx" format, value is set to NN, xCount parameter is
// set to the number of 'x'. Parse +-000 as +-1000, to distinguish +-0.
const char *AgentCfg::ParseCustomStatsScopeValue(const String &str, int &value, int &xCount) {
	if (!str)
		return "empty string";

	bool negative = false;
	const char *p = str.cstr();
	const char *const end = p + str.len();
	if (!isdigit(*p)) {
		if (*p == '-')
			negative = true;
		else if (*p != '+')
			return "must start with DIGIT, '+' or '-'";
		++p;
	}

	if (end - p != 3)
		return "bad length";

	if (!isInt(p, value, &p))
		return "must start with [+-]DIGIT";

	xCount = end - p;
	Must(0 <= xCount && xCount <= 2);

	if (xCount > 0 && !str.endsWith("xx", xCount)) {
		return xCount == 1 ?  "expected a single 'x' at the end" :
			"expected 'xx' at the end";
	}

	// store 0 value as 1000 (impossible in other cases) to distinguish +-0
	if (!value)
		value = NumberOfHttpStatusCodes;

	if (negative)
		value = -value;

	return 0;
}

// values points to a C array with 3 members for NNN, NNx, and Nxx codes
void AgentCfg::ParseCustomStatsScope(const AgentSym *const agent, const Array<StringSym*> &syms, Array<int> *values, bool &allSeen) {
	bool positiveSeen = false;
	for (int i = 0; i < syms.count(); ++i) {
		const String &str = syms[i]->val();
		if (str == "all") {
			allSeen = true;
			continue;
		}

		int value = 0;
		int xCount = 0;
		if (const char *err = ParseCustomStatsScopeValue(str, value, xCount)) {
			cerr << agent->loc() << "invalid custom_stats_scope value \""
				<< str << "\": " << err << endl << xexit;
		}
		if (value > 0 && !positiveSeen)
			positiveSeen =  true;

		Must(0 <= xCount && xCount <= 2);
		values[xCount].append(value);
	}

	if (!allSeen && !positiveSeen) {
		Comment(1) << agent->loc() << "no positive values in custom_stats_scope,"
			" assume implicit \"all\"" << endc;
		allSeen = true;
	}
}

void AgentCfg::setCustomStatsScope(int value, const int range) {
	const bool included = value > 0;
	value = Abs(value);
	if (value == NumberOfHttpStatusCodes)
		value = 0;
	else
		value *= range;
	for (int i = value; i < value + range; ++i)
		theCustomStatsScope.set(i, included);
}

void AgentCfg::configureCustomStatsScope(const AgentSym *const agent) {
	Array<StringSym*> syms;
	if (!agent->customStatsScope(syms))
		return;

	const int maxXcount = 3;
	Array<int> values[maxXcount]; // Arrays for NNN, NNx and Nxx codes
	for (int i = 0; i < maxXcount; ++i)
		values[i].stretch(syms.count());

	bool allSeen = false;
	ParseCustomStatsScope(agent, syms, values, allSeen);
	if (allSeen)
		theCustomStatsScope.set();

	for (int i = maxXcount - 1; i >= 0; --i) {
		for (int j = 0; j < values[i].count(); ++j) {
			static const int ranges[] = {1, 10, 100};
			setCustomStatsScope(values[i][j], ranges[i]);
		}
	}
}
