
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/LoadableModule.h"
#include "xstd/gadgets.h"
#include "xstd/rndDistrs.h"
#include "base/OLog.h"
#include "base/polyLogCats.h"
#include "runtime/LogComment.h"
#include "runtime/SharedOpts.h"
#include "runtime/StatIntvl.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/Rptmstat.h"
#include "runtime/DutWatchdog.h"
#include "runtime/ErrorMgr.h"
#include "runtime/PersistWorkSetMgr.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "runtime/globals.h"
#include "runtime/HttpCookies.h"
#include "runtime/ObjUniverse.h"
#include "pgl/PglStaticSemx.h"
#include "pgl/RobotSym.h"
#include "app/BeepDoorman.h"
#include "client/AsyncClt.h"
#include "client/SyncClt.h"
#include "client/PassClt.h"
#include "client/ServerRep.h"
#include "client/IcpCltXact.h"
#include "client/CltOpts.h"
#include "client/Client.h"
#include "client/FtpCltXact.h"
#include "client/HttpCltXact.h"
#include "client/CltConnMgr.h"
#include "client/SessionWatchRegistry.h"
#include "client/CltDataFilterRegistry.h"
#include "client/wssFreezers.h"
#include "client/PolyClt.h"
#include "runtime/XactFarm.h"

typedef XactFarmT<CltXact, FtpCltXact> FtpCltXactFarm;
typedef XactFarmT<CltXact, HttpCltXact> HttpCltXactFarm;


PolyClt::PolyClt() {
	theAgentType = RobotSym::TheType;
	theChannels.append(TheSessionBegChannel);
	theChannels.append(TheSessionCntChannel);
	theChannels.append(TheSessionEndChannel);
	theChannels.append(TheInfoChannel);
}

PolyClt::~PolyClt() {
	TheSessionWatchRegistry().clear();
	TheCltDataFilterRegistry().clear();
	while (theModules.count())
		delete theModules.pop(); // will unload
}

Agent *PolyClt::makeAgent(const AgentSym &agent, const NetAddr &address) {
	const RobotSym *rcfg = &(const RobotSym&)agent.cast(RobotSym::TheType);

//	cerr << here << "address=" << address << endl;

	Client *client = 0;
	RndDistr *iad = 0;
	if (rcfg->reqInterArrival(iad)) {
		if (iad)
			client = new AsyncClt(iad);
		else
			client = new PassClt();
	} else
		client = new SyncClt();

	Assert(client);
	client->configure(rcfg, address);

	addAgent(client);
	return client;
}

void PolyClt::configure() {

	PolyApp::configure();

	loadModules(TheCltOpts.theLoadableModules.val());

	TheWssFreezer = 0;
	if (PglStaticSemx::TheWorkSetLen >= 0 && PglStaticSemx::TheWorkSetCap >= 0) {
		cerr << ThePrgName << ": canot handle more than one WSS freezing conditions" << endl;
		exit(-3);
	}
	if (PglStaticSemx::TheWorkSetLen >= 0)
		TheWssFreezer = new TimeWssFreezer(PglStaticSemx::TheWorkSetLen);
	if (PglStaticSemx::TheWorkSetCap >= 0)
		TheWssFreezer = new FillWssFreezer(PglStaticSemx::TheWorkSetCap);

	Rptmstat::IsEnabled = true; // only clinet-side cares about load
	DutWatchdog::IsEnabled = true; // only clinet-side cares about load

	TheCltBehaviorCfgs.configure();
	Client::FtpFarm(new FtpCltXactFarm);
	Client::HttpFarm(new HttpCltXactFarm);
	IcpCltXact::TheTimeout = TheCltOpts.theIcpTout;
	StatIntvl::ActiveCat(lgcCltSide);
	StatIntvl::TheReportCat = lgcCltSide;
	HttpCookie::Configure();
}

void PolyClt::getOpts(Array<OptGrp*> &opts) {
	PolyApp::getOpts(opts);
	opts.append(&TheCltOpts);
}

void PolyClt::reportCfg() {
	PolyApp::reportCfg();

	TheSessionWatchRegistry().report(Comment);
	Comment << endc;
	TheCltDataFilterRegistry().report(Comment);
	Comment << endc;
}

void PolyClt::loadPersistence() {
	PolyApp::loadPersistence();
	if (ThePersistWorkSetMgr.loadSideState())
		ObjUniverse::ReportWss(1);
}

void PolyClt::loadModules(const Array<String*> &names) {
	for (int i = 0; i < names.count(); ++i) {
		const String &name = *names[i];
		LoadableModule *module = new LoadableModule(name);
		Comment(7) << "loading dynamic module: " << name << endc;
		if (!module->load()) {
			Comment << "error loading dynamic module '" << name << "': " 
				<< module->error() << endc;
			FatalError(errOther);
		}
		theModules.append(module);
	}
	Comment(1) << "dynamic modules loaded: " << theModules.count() << endc;
}

void PolyClt::logState(OLog &log) {
	PolyApp::logState(log);
	Client::LogState(log);
}

void PolyClt::startAgents() {
	Comment(1) << "fyi: max local population size: " << theLocals.count() << " robots" << endc;

	// do not start robots here; robots are started using population factor
	theAvailClts.stretch(theLocals.count());
	for (int i = 0; i < theLocals.count(); ++i)
		theAvailClts.append((Client*)theLocals[i]);
}

void PolyClt::startClients(int count) {
	const int wasAvailableCnt = theAvailClts.count();

	while (theAvailClts.count() && count-- > 0) {
		Client *clt = flipCltState(theAvailClts, thePopulus);
		clt->start();
	}

	if (count > 0 && ReportError(errUnderpopulated)) {
		Comment << "cannot increase population size;"
			<< " current level: " << thePopulus.count()
			<< " robots available: " << theAvailClts.count()
			<< " debt: " << count << endc;
	} else
	if (wasAvailableCnt && !theAvailClts.count()) {
		Comment(3) << "fyi: reached max local population size: " 
			<< thePopulus.count() << " robots" << endc;
	}
}

void PolyClt::stopClients(int count) {
	while (thePopulus.count() && count-- > 0) {
		Client *clt = flipCltState(thePopulus, theAvailClts);
		clt->stop();
	}
	// if count > 0, then there were rounding errors in count calculations?

	if (thePopulus.count() <= 1) {
		Comment(3) << "fyi: reached min local population size: " 
			<< thePopulus.count() << " robots" << endc;
	}
}

Client *PolyClt::flipCltState(Clients &from, Clients &to) {
	Assert(from.count());
	static RndGen rng;
	const int idx = rng(0, from.count());
	
	Client *clt = from[idx];
	from.eject(idx);
	to.append(clt);

	return clt;
}

void PolyClt::startServices() {
	PolyApp::startServices();

	// assume client traffic starts right away, no need to wait
	if (TheStatPhaseMgr.trafficWaiting())
		TheStatPhaseMgr.stopTrafficWaiting();

	if (TheWssFreezer)
		TheWssFreezer->start();
}

void PolyClt::step() {
	/* check active populus levels */

	const double pf = TheStatPhaseMgr->populusFactor().current();

	const int goal = pf > 0 ? // leave at least one client alive if pf > 0
		Max(1, (int)rint(pf*theLocals.count())) : 0;
	const int diff = goal - thePopulus.count();

	if (diff < 0)
		stopClients(-diff);
	else
	if (diff > 0)
		startClients(diff);

	PolyApp::step();
}

const String PolyClt::sideName() const {
	return "client";
}

int PolyClt::logCat() const {
	return lgcCltSide;
}

void PolyClt::noteClientEvent(BcastChannel *ch, const Client *c) {
	Assert(c);
	String status;
	if (ch == TheSessionBegChannel) {
		status = "begin";
	} else
	if (ch == TheSessionCntChannel) {
		status = "continue";
	} else
	if (ch == TheSessionEndChannel) {
		status = "end";
	} else {
		Assert(false);
	}

	if (theBeepDoorman) {
		ostringstream buf;
		buf
			<< "<session user='" << c->credentials() << "' "
			<< "address='" << c->host() << "' "
			<< "status='" << status << "' />"
			<< ends;
		theBeepDoorman->bcastMsg(buf.str().c_str());
		streamFreeze(buf, false);
	}
}

void PolyClt::noteInfoEvent(BcastChannel *ch, InfoEvent ev) {
	Assert(ch == TheInfoChannel);
	if (ev == ieReportProgress)
		ObjUniverse::ReportWss(7);
}

int main(int argc, char *argv[]) {
	PolyClt app;
	return app.run(argc, argv);
}
