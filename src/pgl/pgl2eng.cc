
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "pgl/pgl.h"

#include "xstd/Rnd.h"
#include "xstd/gadgets.h"
#include "base/polyVersion.h"
#include "xml/XmlAttr.h"
#include "xml/XmlText.h"
#include "xml/XmlParagraph.h"
#include "xml/XmlTable.h"
#include "xml/XmlSection.h"
#include "pgl/PglPp.h"
#include "pgl/PglCtx.h"
#include "pgl/PglStaticSemx.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglDistrSym.h"
#include "pgl/AgentSymIter.h"
#include "pgl/GoalSym.h"
#include "pgl/PhaseSym.h"
#include "pgl/MimeSym.h"
#include "pgl/ContentSym.h"
#include "pgl/PopDistr.h"
#include "pgl/PopModelSym.h"
#include "pgl/ServerSym.h"
#include "pgl/RobotSym.h"


static String ThePrgName;

struct PglParser {}; // stub for future use

static
ostream &addAnd(ostream &os) {
	if (os.tellp())
		os << ", ";
	return os;
}

static
ostream &addAnd(ostream &os, int idx, int count) {
	if (!idx)
		return os;

	if (idx == count-1)
		return os << (count == 2 ? " and " : ", and ");

	return os << ", ";
}

static
ostream &addOneOrMany(ostream &os, int count, const String &oneStr, const char *manyStr = 0) {
	if (count == 1)
		return os << oneStr;
	
	if (manyStr)
		return os << manyStr;

	// XXX: 'h' only if 'sh' and 'zh';
	return os << oneStr << (strchr("szjh", oneStr.last()) ? "es" : "s");	
}

static
double phaseFactor(const Array<PhaseSym*> &phases, int idx, PhaseSym::FactorFunc fbeg, PhaseSym::FactorFunc fend, bool checkBeg) {
	if (idx < 0)
		return 1;
	const PhaseSym *ph = phases[idx];
	const PhaseSym::FactorFunc fcur = checkBeg ? fbeg : fend;
	double fact = 1;
	if ((ph->*fcur)(fact))
		return fact;

	if (checkBeg)
		return phaseFactor(phases, idx-1, fbeg, fend, false);
	else
		return phaseFactor(phases, idx, fbeg, fend, true);
}

static
void buildPhaseTable(XmlTable &table, const Array<PhaseSym*> &phases) {
	static PhaseSym::FactorFunc funcs[6] = {
		&PhaseSym::populusFactorBeg, &PhaseSym::populusFactorEnd,
		&PhaseSym::recurFactorBeg, &PhaseSym::recurFactorEnd,
		&PhaseSym::specialMsgFactorBeg, &PhaseSym::specialMsgFactorEnd
	};

	table << XmlAttr::Int("border", 1) << XmlAttr("bgcolor", "#FFFFFF");

	XmlTableRec r1, r2, r3;
	r1
		<< XmlTableHeading("Phase", 1, 3)
		<< XmlTableHeading("Factors (%)", 3*2, 1)
		<< XmlTableHeading("Other", 1, 3);

	r2
		<< XmlTableHeading("Populus", 2, 1)
		<< XmlTableHeading("Recurrence", 2, 1)
		<< XmlTableHeading("Special Msgs", 2, 1);
		
	for (int c = 0; c < 3; ++c) {
		r3 << XmlTableHeading("beg") << XmlTableHeading("end");
	}

	XmlTableHeader header;
	header << r1 << r2 << r3;
	header << XmlAttr("bgcolor", "#BBBBBB");
	table << header;

	// we need higher precision if at least one cell needs it
	bool highPrec = false;
	{for (int i = 0; !highPrec && i < phases.count(); ++i) {
		for (int f = 0; !highPrec && f < 6; ++f) {
			const double x = 100*(f % 2 ?
				phaseFactor(phases, i, funcs[f-1], funcs[f], false) :
				phaseFactor(phases, i, funcs[f], funcs[f+1], true));
			highPrec = floor(x) < floor(x + 0.999);
		}
	}}

	for (int i = 0; i < phases.count(); ++i) {
		XmlTableRec rec;
		const PhaseSym &phase = *phases[i];

		rec << XmlTableHeading(phase.name());

		{for (int f = 0; f < 6; ++f) {
			const double x = 100*(f % 2 ?
				phaseFactor(phases, i, funcs[f-1], funcs[f], false) :
				phaseFactor(phases, i, funcs[f], funcs[f+1], true));

			XmlTextTag<XmlTableCell> cell;
			cell << XmlAttr("align", "right");
			if (highPrec)
				cell.buf() << x;
			else
				cell.buf() << (int)x;
			rec << cell;
		}}

		XmlTextTag<XmlTableCell> cell;
		if (phase.rptmstat())
			cell.buf() << addAnd << "rptmstat enabled";

		bool ls = false;
		if (phase.logStats(ls))
			cell.buf() << addAnd << (ls ? "" : "do not ") << "log stats";

		bool wwf = false;
		if (phase.waitWssFreeze(wwf) && addAnd(cell.buf()))
			cell.buf() << addAnd << (wwf ? "" : "do not ") << "wait for WSS to freeze";

		if (!cell.buf().tellp())
			cell.buf() << "&nbsp;";

		rec << cell;
		table << rec;
	}
}

static
Time testDuration(const Array<PhaseSym*> &phases, int &fixCount, int &varCount) {
	fixCount = varCount = 0;
	Time dur(0,0);

	for (int i = 0; i < phases.count(); ++i) {
		if (GoalSym *gs = phases[i]->goal()) {
			const Time d = gs->duration();
			if (d >= 0) {
				fixCount++;
				dur += d;
			}
		}
	}
	varCount = phases.count() - fixCount;
	return dur;
}

static
const char *changeStr(double prev, double cur) {
	return prev <= cur ? "increases" : "decreases";
}

static
void explainPhase(XmlTag &sect, const Array<PhaseSym*> &phases, int idx) {
	const PhaseSym &phase = *phases[idx];
	XmlTextTag<XmlParagraph> p;

	const Time dur = phase.goal() ? phase.goal()->duration() : Time();

	if (dur >= 0)
		p.buf() << "Phase \"" << phase.name() << "\" lasts for " << dur << ". ";
	else
		p.buf() << "Phase \"" << phase.name() << "\" does not have a time-based duration configured. ";

	double prev, cur;
	double d;

	prev = phaseFactor(phases, idx, &PhaseSym::populusFactorBeg, &PhaseSym::populusFactorEnd, true);
	cur = phaseFactor(phases, idx, &PhaseSym::populusFactorBeg, &PhaseSym::populusFactorEnd, false);
	if (!phase.populusFactorEnd(d)) {
		p.buf() << "During this phase, the robot population size remains "
			<< "stable at " << (cur*100) << "% of its peak level. ";
	} else {
		p.buf() << "During this phase, the robot population size " << changeStr(prev, cur)
			<< " from " << (prev*100) << "% to " << (cur*100) << "%. ";
	}

	prev = phaseFactor(phases, idx, &PhaseSym::loadFactorBeg, &PhaseSym::loadFactorEnd, true);
	cur = phaseFactor(phases, idx, &PhaseSym::loadFactorBeg, &PhaseSym::loadFactorEnd, false);
	if (!phase.loadFactorEnd(d)) {
		p.buf() << "The offered per-robot load remains "
			<< "stable at " << (cur*100) << "% of its peak level. ";
	} else {
		p.buf() << "The offered per-robot load level " << changeStr(prev, cur)
			<< " from " << (prev*100) << "% to " << (cur*100) << "%. ";
	}

	if (phase.rptmstat()) {
		p.buf() << "However, load level is also subject to response time "
			<< "constraints since rptmstat controls are enabled. ";
		// XXX: report rptmstat config
	}

	prev = phaseFactor(phases, idx, &PhaseSym::recurFactorBeg, &PhaseSym::recurFactorEnd, true);
	cur = phaseFactor(phases, idx, &PhaseSym::recurFactorBeg, &PhaseSym::recurFactorEnd, false);
	if (!phase.recurFactorEnd(d)) {
		p.buf() << "The recurrence level remains "
			<< "stable at " << (cur*100) << "% of robot recurrence ratios. ";
	} else {
		p.buf() << "The offered recurrence level " << changeStr(prev, cur) 
			<< " from " << (prev*100) << "% to " << (cur*100) 
			<< "% of robot recurrence ratios. ";
	}

	prev = phaseFactor(phases, idx, &PhaseSym::specialMsgFactorBeg, &PhaseSym::specialMsgFactorEnd, true);
	cur = phaseFactor(phases, idx, &PhaseSym::specialMsgFactorBeg, &PhaseSym::specialMsgFactorEnd, false);
	if (!phase.specialMsgFactorEnd(d)) {
		p.buf() << "The portion of special messages remains "
			<< "stable at " << (cur*100) << "%. ";
	} else {
		p.buf() << "The portion of special messages changes " << changeStr(prev, cur)
			<< " from " << (prev*100) << "% to " << (cur*100) << "%. ";
	}


	Array<const StatsSampleSym*> samples;
	if (phase.statsSamples(samples)) {
		p.buf() << samples.count() << " samples of per-transaction"
			<< " statistics are collected. ";
	}

	bool doIt;
	if (phase.logStats(doIt) && !doIt) {
		p.buf() << "No phase statistics is logged for this phase. ";
	}

	if (phase.waitWssFreeze(doIt) && doIt) {
		p.buf() << "The phase will continue until working set size is frozen. ";
	}

	sect << p;
}

static
void explainPhases(XmlNodes &code, const PglParser &) {
	const Array<PhaseSym*> &phases = PglStaticSemx::TheSchedule;

	XmlSection sect(2, "Phase schedule");

	if (phases.count()) {
		XmlTextTag<XmlParagraph> p;

		p.buf() << "The workload schedule consists of " << phases.count();
		addOneOrMany(p.buf(), phases.count(), " phase") << ". ";

		int fixDurCount = 0;
		int varDurCount = 0;
		const Time fixedDur = testDuration(phases, fixDurCount, varDurCount);
		p.buf() << "The schedule includes " << fixDurCount << " phases "
			<< "with time-based goals";
		if (varDurCount)
			addOneOrMany(p.buf() << " and " << varDurCount << " other ", varDurCount, "phase");
		p.buf() << ". ";
		if (fixedDur > 0) {
			p.buf() << "The total test duration (based on the"
				<< " time-based goals) is about " << fixedDur << ". ";
		}
		sect << p;

		XmlTag c("center");
		XmlTable table;
		buildPhaseTable(table, phases);
		c << table;
		sect << c;

		for (int i = 0; i < phases.count(); ++i) {
			explainPhase(sect, phases, i);
		}
	} else {
		XmlTextTag<XmlParagraph> p;
		p.buf() << "The workload does not define a schedule. "
			<< "Polygraph will use a default schedule consisting of a "
			<< "single phase with no goal and default factors. ";
		sect << p;
	}

	code << sect;
}

static
void explainAgent(XmlNodes &code, const AgentSym &agent, const String &type) {
	XmlText t;

	if (const RndDistr *ul = agent.pconnUseLmt()) {
		t.buf() <<  "This " << type << " uses persistent connections. "
			<< "The number of transactions per connection is distributed as ";
		ul->print(t.buf(), &DistrSym::IntArgPrinter) << ". ";
		const Time tout = agent.idlePconnTimeout();
		if (tout >= 0) {
			t.buf() << "Idle persistent connections are closed after a "
				<< tout << " timeout. ";
		} else {
			t.buf() << "Idle persistent connections are never closed by this "
				<< type << ". ";
		}
	} else {
		t.buf() << "This " << type << " does not use persistent connections. ";
	}

	code << t;
}

static
void explainMsgTypes(XmlNodes &code, const AgentSym &agent, const String &msgKind) {
	XmlText t;
	Array<StringSym*> msgTypes;
	Array<double> tprobs;
	if (agent.msgTypes(msgTypes, tprobs)) {
		t.buf() << "The following " << msgTypes.count() << ' ' << msgKind << ' ';
		addOneOrMany(t.buf(), msgTypes.count(), "type is", "types are")
			<< " used: ";

		for (int i = 0; i < msgTypes.count(); ++i) {
			addAnd(t.buf(), i, msgTypes.count());
			t.buf() << '"' << msgTypes[i]->val() << '"' 
				<< " (" << (100*tprobs[i]) << "%";
			if (!i)
				t.buf() << " of all possible " << msgKind << " types";
			t.buf() << ")";
		}
		t.buf() << ". ";

	} else {
		t.buf() << "Only basic " << msgKind << " types are used. ";
	}
	code << t;
}

static
void explainServer(XmlTag &sect, const ServerSym &srv) {
	XmlTextTag<XmlParagraph> p;

	Array<ContentSym*> ccfgs;
	Array<double> cprobs;
	if (srv.contents(ccfgs, cprobs)) {
		p.buf() << "Server \"" << srv.kind() << "\" hosts the following "
			<< ccfgs.count() << " content ";
		addOneOrMany(p.buf(), ccfgs.count(), "type") << ": ";

		for (int i = 0; i < ccfgs.count(); ++i) {
			addAnd(p.buf(), i, ccfgs.count());
			p.buf() << '"' << ccfgs[i]->kind() << '"' 
				<< " (" << (100*cprobs[i]) << "%";
			if (!i)
				p.buf() << " of all hosted content";
			p.buf() << ")";
		}
		p.buf() << ". ";

	} else {
		p.buf() << "Server type \"" << srv.kind() << "\" does not host "
			" any content!";
	}

	Array<ContentSym*> dacfgs;
	Array<double> daprobs;
	if (srv.directAccess(dacfgs, daprobs)) {
		p.buf() << "The following "
			<< dacfgs.count() << " content ";
		addOneOrMany(p.buf(), dacfgs.count(), "type") << 
			" can be accessed directly: ";

		for (int i = 0; i < dacfgs.count(); ++i) {
			addAnd(p.buf(), i, dacfgs.count());
			p.buf() << '"' << dacfgs[i]->kind() << '"';
		}
		p.buf() << ". ";

	} else {
		p.buf() << "No hosted content can be accessed direclty. ";
	}

	if (RndDistr *xt = srv.xactThink()) {
		p.buf() << "Server \"think time\" distribution is set to ";
		xt->print(p.buf(), &DistrSym::TimeArgPrinter) << ". ";
	} else {
		p.buf() << "The server responds to all requests without"
			<< " artificial delays. "; 
	}

	XmlNodes code;
	explainAgent(code, srv, "server");
	explainMsgTypes(code, srv, "reply");
	p << code;

	sect << p;
}

static
void explainServers(XmlNodes &code, const PglParser &) {
	XmlSection sect(2, "Servers configuration");

	Array<const ServerSym *> servers;
	for (AgentSymIter i(PglStaticSemx::TheAgentsToUse, "Server", false); i; ++i)
		servers.append(&(ServerSym&)i.agent()->cast("Server"));

	if (servers.count()) {
		XmlTextTag<XmlParagraph> p;
		p.buf() << "The workload defines " << servers.count() << " server ";
		addOneOrMany(p.buf(), servers.count(), "type") << ". ";
		sect << p;

		for (int i = 0; i < servers.count(); ++i) {
			explainServer(sect, *servers[i]);
		}
	} else {
		XmlTextTag<XmlParagraph> p;
		p.buf() << "The workload does not use any servers. ";
		sect << p;
	}

	code << sect;
}

static
void explainInterests(XmlNodes &code, const RobotSym &robot) {
	XmlText t;
	Array<StringSym*> interests;
	Array<double> iprobs;
	if (robot.interests(interests, iprobs)) {
		t.buf() << "The robot is interested in the following " 
			<< interests.count() << " object groups: ";

		for (int i = 0; i < interests.count(); ++i) {
			addAnd(t.buf(), i, interests.count());
			t.buf() << '"' << interests[i]->val() << '"' 
				<< " (" << (100*iprobs[i]) << "%";
			if (!i)
				t.buf() << " of all requests";
			t.buf() << ")";
		}
		t.buf() << ". ";
	} else {
		t.buf() << "The robot only requests private objects (i.e., " 
			<< "it does not share request URLs with other robots).";
	}
	code << t;
}

static
void explainRobot(XmlTag &sect, const RobotSym &rbt) {
	XmlTextTag<XmlParagraph> p1;

	RndDistr *iad = 0;
	if (rbt.reqInterArrival(iad)) {
		if (iad) {
			p1.buf() << "Robot \"" << rbt.kind() << "\" is a \"constant "
				<< "request rate\" robot ";
			if (String(iad->pdfName()) == "exp") {
				const double rate = 1/iad->mean();
				p1.buf() << "with request rate of " << rate
					<< " requests per second. ";
			} else {
				p1.buf() << "with request inter-arrival distribution set to ";
				iad->print(p1.buf(), &DistrSym::TimeArgPrinter) << ". ";
			}
		} else {
			p1.buf() << "Robot \"" << rbt.kind() << "\" is a \"passive\" "
				<< "robot that will not submit any requests on its own. ";
		}
	} else {
		p1.buf() << "Robot \"" << rbt.kind() << "\" is a \"best-effort\" "
				<< "robot that will start next HTTP transaction as soon "
				<< "as the previous transaction completes. ";
	}

	double ratio;

	if (rbt.recurRatio(ratio)) {
		p1.buf() << "This robot revisits " << (100*ratio) << "% of previously "
			<< "requested URLs (offering a hit when a URL is cachable). ";
	}

	if (rbt.embedRecurRatio(ratio)) {
		p1.buf() << "About " << (100*ratio) << "% of embedded objects "
			<< " will be loaded. ";
	}

	if (rbt.minimizeNewConn(ratio)) {
		p1.buf() << "The robot will attempt to minimize the number of new "
			<< "connections (by using substitutes servers) with a probability "
			<< "of " << (100*ratio) << "%. ";
	}

	sect << p1;

	XmlTextTag<XmlParagraph> p2;

	int lmt;
	if (rbt.openConnLimit(lmt)) {
		p2.buf() << "This robot is not allowed to open more than " << lmt;
		addOneOrMany(p2.buf(), lmt, " connection") << " at any given time, "
			<< "even if that limit causes decrease in request rate or "
			<< "memory exhaustion. ";

		if (rbt.waitXactLimit(lmt)) {
			p2.buf() << "However, the robot limits waiting transaction queue ";
			addOneOrMany(p2.buf() << "to " << lmt, lmt, " transaction") 
				<< ", cancelling newly submitted transactions as needed. ";
		} else {
			p2.buf() << "Moreover, waiting transaction queue can grow without bounds. ";
		}

	} else {
		p2.buf() << "This robot opens as many connections as needed. ";
	}

	if (rbt.privCache(lmt)) {
		p2.buf() << "Robot's private cache is limited to " << lmt;
		addOneOrMany(p2.buf(), lmt, " entry", " entries") << ". ";
	}

	XmlNodes code;
	explainAgent(code, rbt, "robot");
	explainInterests(code, rbt);
	explainMsgTypes(code, rbt, "request");

	p2 << code;
	sect << p2;


	XmlTextTag<XmlParagraph> p3;

	if (PopModelSym *pm = rbt.popModel()) {
		const PopDistr *pd = pm->popDistr();
		double hotSetFrac;
		double hotSetProb;
		if (pd && pm->hotSetFrac(hotSetFrac) && pm->hotSetProb(hotSetProb)) {
			p3.buf() << "\"" << rbt.kind() << "\" robots direct "
				<< (100*hotSetProb) << "% of all requests to "
				<< (100*hotSetFrac) << "% of the working set, using ";
			pd->print(p3.buf()) << " popularity distribution. ";
		}
	}

	sect << p3;
}

static
void explainRobots(XmlNodes &code, const PglParser &) {
	XmlSection sect(2, "Robots configuration");

	Array<const RobotSym *> robots;
	for (AgentSymIter i(PglStaticSemx::TheAgentsToUse, RobotSym::TheType, false); i; ++i)
		robots.append(&(RobotSym&)i.agent()->cast(RobotSym::TheType));

	if (robots.count()) {
		XmlTextTag<XmlParagraph> p;
		p.buf() << "The workload defines " << robots.count() << " robot ";
		addOneOrMany(p.buf(), robots.count(), "type") << ". ";
		sect << p;

		for (int i = 0; i < robots.count(); ++i) {
			explainRobot(sect, *robots[i]);
		}
	} else {
		XmlTextTag<XmlParagraph> p;
		p.buf() << "The workload does not use any robots. ";
		sect << p;
	}

	code << sect;
}


static
void explainContType(XmlTag &sect, const ContentSym &ctype) {
	XmlTextTag<XmlParagraph> p;

	if (RndDistr *sized = ctype.size()) {
		p.buf() << "The size distribution for \"" << ctype.kind() 
			<< "\" content type is ";
		sized->print(p.buf(), &DistrSym::SizeArgPrinter);
		p.buf() << ". ";
	} else {
		p.buf() << "Content type \"" << ctype.kind() << "\" has unknown size. ";
	}

	double ratio = 0;
	ctype.cachable(ratio); // default is 0
	p.buf() << "About " << (100*ratio) << "% of \"" << ctype.kind() 
		<< "\" objects are cachable. ";

	Array<ContentSym*> ccfgs;
	RndDistr *selector;
	Array<double> cprobs;
	if (ctype.contains(ccfgs, selector, cprobs) && ccfgs.count()) {
		p.buf() << "This content type is a container. Objects may "
			<< "contain (embed) the following "	<< ccfgs.count();
		addOneOrMany(p.buf(), ccfgs.count(), " content type") << ": ";

		for (int i = 0; i < ccfgs.count(); ++i) {
			addAnd(p.buf(), i, ccfgs.count());
			p.buf() << '"' << ccfgs[i]->kind() << '"' 
				<< " (" << (100*cprobs[i]) << "%";
			if (!i)
				p.buf() << " of all embedded content";
			p.buf() << ")";
		}
		p.buf() << ". ";

		if (RndDistr *embCnt = ctype.embedCount()) {
			p.buf() << "The number of embedded objects per container is "
				<< "distributed as ";
			embCnt->print(p.buf(), &DistrSym::IntArgPrinter) << ". ";
		}
	} else {
		p.buf() << "This content type does not contain other types. ";
	}

	if (MimeSym *mime = ctype.mime()) {

		const String mtype = mime->mimeType();
		if (mtype) {
			p.buf() << "Generated object have \"" << mtype 
				<< "\" MIME type. ";
		}

		Array<String*> pfxs;
		RndDistr *sel;
		if (mime->prefixes(pfxs, sel) && pfxs.count()) {
			p.buf() << "URLs may start with the following " << pfxs.count();
			addOneOrMany(p.buf(), pfxs.count(), " string") << ": ";
			for (int x = 0; x < pfxs.count(); ++x) {
				addAnd(p.buf(), x, pfxs.count());
				p.buf() << '"' << *pfxs[x] << '"';
			}
			p.buf() << ". ";
		}

		Array<String*> exts;
		if (mime->extensions(exts, sel) && exts.count()) {
			p.buf() << "The following " << exts.count();
			addOneOrMany(p.buf(), exts.count(), " extension") 
				<< " may appear at the end of URLs: ";
			for (int e = 0; e < exts.count(); ++e) {
				addAnd(p.buf(), e, exts.count());
				p.buf() << '"' << *exts[e] << '"';
			}
			p.buf() << ". ";
		}
	}

	sect << p;
}

static
void buildContTypeTable(XmlTable &table, const Array<const ContentSym*> &ctypes) {

	table << XmlAttr::Int("border", 1) << XmlAttr("bgcolor", "#FFFFFF");

	XmlTableRec hr;
	hr
		<< XmlTableHeading("Type")
		<< XmlTableHeading("Reply Size")
		<< XmlTableHeading("Cachability")
		<< XmlTableHeading("Extensions");
	
	XmlTableHeader header;
	header << hr;
	header << XmlAttr("bgcolor", "#BBBBBB");
	table << header;

	for (int i = 0; i < ctypes.count(); ++i) {
		XmlTableRec rec;
		const ContentSym &ctype = *ctypes[i];

		rec << XmlTableHeading(ctype.kind());

		XmlTextTag<XmlTableCell> cellSize;
		if (RndDistr *sized = ctype.size())
			sized->print(cellSize.buf(), &DistrSym::SizeArgPrinter);
		else
			cellSize.buf() << '?';
		cellSize << XmlAttr("align", "center");
		rec << cellSize;

		XmlTextTag<XmlTableCell> cellChb;
		double ratio = 0;
		ctype.cachable(ratio);
		cellChb.buf() << (100*ratio) << '%'; // default is zero
		cellChb << XmlAttr("align", "right");
		rec << cellChb;

		XmlTextTag<XmlTableCell> cellExts;
		if (MimeSym *mime = ctype.mime()) {
			Array<String*> exts;
			RndDistr *sel;
			if (mime->extensions(exts, sel)) {
				for (int e = 0; e < exts.count(); ++e) {
					addAnd(cellExts.buf(), e, exts.count());
					cellExts.buf() << *exts[e];
				}
			}
		}
		if (!cellExts.buf().tellp())
			cellExts.buf() << "&nbsp;";
		rec << cellExts;

		table << rec;
	}
}

static
void explainContTypes(XmlNodes &code, const PglParser &) {
	XmlSection sect(2, "Content types");

	Array<const ContentSym*> ctypes;
	{for (AgentSymIter i(PglStaticSemx::TheAgentsToUse, ServerSym::TheType, false); i; ++i) {
		const ServerSym &srv = (const ServerSym&)i.agent()->cast(ServerSym::TheType);
		Array<ContentSym*> ccfgs;
		Array<double> cprobs;
		if (srv.contents(ccfgs, cprobs)) {
			for (int c = 0; c < ccfgs.count(); ++c) {
				bool found = false;
				for (int f = 0; !found && f < ctypes.count(); ++f)
					found = ctypes[f]->equal(*ccfgs[c]);
				if (!found)
					ctypes.append(ccfgs[c]);
			}
		}
	}}

	if (ctypes.count()) {
		XmlTextTag<XmlParagraph> p;

		p.buf() << "The workload uses " << ctypes.count();
		addOneOrMany(p.buf(), ctypes.count(), " unique content type") << ". ";
		sect << p;

		XmlTag c("center");
		XmlTable table;
		buildContTypeTable(table, ctypes);
		c << table;
		sect << c;

		for (int i = 0; i < ctypes.count(); ++i) {
			explainContType(sect, *ctypes[i]);
		}
	} else {
		XmlTextTag<XmlParagraph> p;
		p.buf() << "The workload does not use any content types. ";
		sect << p;
	}

	code << sect;
}

static
int usage(std::ostream &os) {
	(void)PolyVersion();
	os << "Usage: " << ThePrgName << " <input_file> [include_dir] ..." << endl;
	return 0;
}

int main(int argc, char **argv) {
	ThePrgName = argv[0];

	if (argc < 2)
		return usage(cerr);

	if (String("--help") == argv[1])
		return usage(cout);

	for (int i = 2; i < argc; ++i)
		PglPp::TheDirs.append(new String(argv[i]));

	PglStaticSemx::Interpret(argv[1]);
	PglParser parser;

	XmlNodes code;
	explainPhases(code, parser);
	explainServers(code, parser);
	explainRobots(code, parser);
	explainContTypes(code, parser);

	configureStream(cout, 2);
	code.print(cout, "");

	return 0;
}
