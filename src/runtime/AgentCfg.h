
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_AGENTCFG_H
#define POLYGRAPH__RUNTIME_AGENTCFG_H

#include <bitset>
#include "xstd/Array.h"

class RndDistr;
class AgentSym;
class SslWrap;
class MimeHeadersCfg;
class StringSym;
class XactAbortCoord;

// agent configuration items that can be shared among multiple agents
class AgentCfg {
	public:
		AgentCfg();
		~AgentCfg();

		void configure(const AgentSym *agent);

		int selectHttpVersion();
		bool selectSslWrap(const SslWrap *&wrap);
		bool selectCookieSenderStatus();
		void selectAbortCoord(XactAbortCoord &coord);

		bool inCustomStatsScope(const int httpStatus) const;

		const MimeHeadersCfg *httpHeaders() const { return theHttpHeaders; }

	protected:
		void configureCustomStatsScope(const AgentSym *const agent);
		void configureSslWraps(const AgentSym *agent);
		void configureHttpVersions(const AgentSym *agent);
		void configureHttpHeaders(const AgentSym *agent);
		void setCustomStatsScope(int value, const int range);

		static const char *ParseCustomStatsScopeValue(const String &str, int &value, int &xCount);
		static void ParseCustomStatsScope(const AgentSym *const agent, const Array<StringSym*> &syms, Array<int> *values, bool &allSeen);

	protected:
		enum { NumberOfHttpStatusCodes = 1000 };
		typedef std::bitset<NumberOfHttpStatusCodes> CustomStatsScope;

		CustomStatsScope theCustomStatsScope; // custom stats scope mask
		Array<SslWrap*> theSslWraps;   // local entries from TheSslWraps
		RndDistr *theSslWrapSel;       // selects an SSL wrapper
		RndDistr *theHttpVersionSel;   // selects HTTP version
		MimeHeadersCfg *theHttpHeaders; // user-configured HTTP headers
		double theCookieSenderProb;    // probability of sending cookies
		double theAbortProb;           // prob of aborting a transaction
};

#endif
