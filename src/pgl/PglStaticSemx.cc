
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/sstream.h"

#include "xparser/TokenSym.h"
#include "xparser/ParsSym.h"

#include "pgl/PglPp.h"
#include "pgl/PglParser.h"

#include "pgl/PglTimeSym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglListSym.h"
#include "pgl/PglContainerSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/AgentSym.h"
#include "pgl/NetPipeSym.h"
#include "pgl/AddrMapSym.h"
#include "pgl/SslWrapSym.h"
#include "pgl/MembershipMapSym.h"
#include "pgl/BenchSym.h"
#include "pgl/PhaseSym.h"
#include "pgl/StatsSampleSym.h"
#include "pgl/PglStaticSemx.h"


Array<AgentSym*> PglStaticSemx::TheAgentsToUse;
Array<NetPipeSym*> PglStaticSemx::TheNetPipesToUse;
Array<AddrMapSym*> PglStaticSemx::TheAddrMapsToUse;
Array<SslWrapSym*> PglStaticSemx::TheSslWrapsToUse;
Array<ContainerSym*> PglStaticSemx::TheAddrSubstsToUse;
Array<MembershipMapSym*> PglStaticSemx::TheMembershipsToUse;
BenchSym *PglStaticSemx::TheBench = 0;

Array<PhaseSym*> PglStaticSemx::TheSchedule;
Array<StatsSampleSym*> PglStaticSemx::TheSmplSchedule;

Time PglStaticSemx::TheWorkSetLen;
int PglStaticSemx::TheWorkSetCap = -1;

static const String strAddrArr = "addr[]";


const String PglStaticSemx::Interpret(const String &fname) {
	PglPp pp(fname);
	PglParser parser(&pp);

	if (const SynSym *s = parser.parse()) {
		PglStaticSemx semx;
		semx.interpret(*s);
		delete s;
	} else {
		cerr << here << "internal error: failed to interpret parsed " <<
			fname << endl << xexit;
	}
	return pp.image();
}

// default implementation complaints and exits
void PglStaticSemx::callProc(const String &cname, const ListSym &args) {
	if (cname == "use") {
		use(args);
	} else
	if (cname == "schedule") {
		schedule(args);
	} else
	if (cname == "note_substitutes") {
		noteSubstitutes(args);
	} else
	if (cname == "working_set_length") {
		checkArgs(cname, 1, args);
		const TimeSym &length = (const TimeSym&)
			extractArg(cname, 0, args, TimeSym::TheType);
		TheWorkSetLen = length.val();
	} else
	if (cname == "working_set_cap") {
		checkArgs(cname, 1, args);
		const IntSym &cap = (const IntSym&)
			extractArg(cname, 0, args, IntSym::TheType);
		TheWorkSetCap = cap.val();
	} else {
		PglSemx::callProc(cname, args);
	}
}

template <class Store, class Item>
inline
void cuse(Store &used, Item *item, bool valid, const char *reason, const TokenLoc &loc) {
	if (valid) {
		used.append(item);
		return;
	}
	cerr << loc << "warning: ignoring use() of " << item->type() <<
		" " << reason << endl;
	delete item; // it was cloned in use()
}

// register things that will be actually used
void PglStaticSemx::use(const ListSym &objects) {
	const TokenLoc &loc = objects.loc();
	for (int i = 0; i < objects.count(); ++i) {
		if (objects[i]->isA(ListSym::TheType))
			use((ListSym &)objects[i]->cast(ListSym::TheType)); // flatten
		else
		if (AgentSym *a = (AgentSym*)objects[i]->clone(AgentSym::TheType))
			cuse(TheAgentsToUse, a, a->hostCount(), "without addresses", loc);
		else
		if (NetPipeSym *p = (NetPipeSym*)objects[i]->clone(NetPipeSym::TheType))
			cuse(TheNetPipesToUse, p, p->hostCount(), "without addresses", loc);
		else
		if (AddrMapSym *m = (AddrMapSym*)objects[i]->clone(AddrMapSym::TheType))
			cuse(TheAddrMapsToUse, m, m->usable(), "without names or addresses", loc);
		else
		if (SslWrapSym *m = (SslWrapSym*)objects[i]->clone(SslWrapSym::TheType))
			TheSslWrapsToUse.append(m);
		else
		if (MembershipMapSym *g = (MembershipMapSym*)objects[i]->clone(MembershipMapSym::TheType))
			TheMembershipsToUse.append(g);
		else
		if (BenchSym *b = (BenchSym*)objects[i]->clone(BenchSym::TheType)) {
			if (TheBench) {
				cerr << objects.loc() << "warning: new bench selected with use()" << endl;
				cerr << b->loc() << "possible location of the new bench declaration" << endl;
				cerr << TheBench->loc() << "possible location of the old bench declaration" << endl;
				delete TheBench;
			}
			TheBench = b;
		} else {
			cerr << objects[i]->loc() << "entry of type '" << 
				objects[i]->type() << "' in 'use()' argument list" << 
				endl << xexit;
		}
	}
}

// register stat phases that will be actually used
void PglStaticSemx::schedule(const ListSym &items) {
	for (int i = 0; i < items.count(); ++i) {
		if (items[i]->isA(ListSym::TheType))
			schedule((ListSym &)items[i]->cast(ListSym::TheType)); // flatten
		else
		if (PhaseSym *p = (PhaseSym*)items[i]->clone(PhaseSym::TheType))
			TheSchedule.append(p);
		else
		if (StatsSampleSym *s = (StatsSampleSym*)items[i]->clone(StatsSampleSym::TheType))
			TheSmplSchedule.append(s);
		else {
			cerr << items[i]->loc() << "entry of type '" << 
				items[i]->type() << "' cannot be scheduled" << endl << xexit;
		}
	}
}

// register agents that will be actually used
void PglStaticSemx::noteSubstitutes(const ListSym &groups) {
	// note: the argument list is not flattened
	for (int i = 0; i < groups.count(); ++i) {
		const SynSym &gs = *groups[i];
		if (ArraySym *a = (ArraySym*)gs.clone(strAddrArr)) {
			TheAddrSubstsToUse.append(a);
		} else {
			cerr << gs.loc() << "entry of type '" << gs.type() << 
				"' in 'note_substitutes()' argument list" << endl << xexit;
		}
	}
}
