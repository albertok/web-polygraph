
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSTATICSEMX_H
#define POLYGRAPH__PGL_PGLSTATICSEMX_H

#include "xstd/Time.h"
#include "pgl/PglSemx.h"

class AgentSym;
class NetPipeSym;
class AddrMapSym;
class SslWrapSym;
class ContainerSym;
class MembershipMapSym;
class BenchSym;
class PhaseSym;
class StatsSampleSym;

// interprets calls affecting static configuration
class PglStaticSemx: public PglSemx {
	public:
		static const String Interpret(const String &fname);

	protected:
		virtual void callProc(const String &cname, const ListSym &args);

		void use(const ListSym &agents);
		void schedule(const ListSym &phases);
		void noteSubstitutes(const ListSym &groups);

	public:
		// used objects
		static Array<AgentSym*> TheAgentsToUse;
		static Array<NetPipeSym*> TheNetPipesToUse;
		static Array<AddrMapSym*> TheAddrMapsToUse;
		static Array<SslWrapSym*> TheSslWrapsToUse;
		static Array<ContainerSym*> TheAddrSubstsToUse;
		static Array<MembershipMapSym*> TheMembershipsToUse;
		static BenchSym *TheBench;  // bench configuration

		// schedules
		static Array<PhaseSym*> TheSchedule;
		static Array<StatsSampleSym*> TheSmplSchedule;

		static Time TheWorkSetLen;  // working set length
		static int TheWorkSetCap;   // working set capacity
};

#endif
