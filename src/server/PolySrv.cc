
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/gadgets.h"
#include "base/OLog.h"
#include "base/polyLogCats.h"
#include "runtime/LogComment.h"
#include "runtime/SharedOpts.h"
#include "runtime/StatIntvl.h"
#include "runtime/globals.h"
#include "pgl/ServerSym.h"
#include "server/FtpSrvXact.h"
#include "server/HttpSrvXact.h"
#include "server/Server.h"
#include "server/SrvOpts.h"
#include "server/PolySrv.h"


typedef XactFarmT<SrvXact, HttpSrvXact> HttpSrvXactFarm;
typedef XactFarmT<SrvXact, FtpSrvXact> FtpSrvXactFarm;


PolySrv::PolySrv() {
	theAgentType = ServerSym::TheType;
}

Agent *PolySrv::makeAgent(const AgentSym &agent, const NetAddr &address) {
	const ServerSym *scfg = &(const ServerSym&)agent.cast("Server");
	Server *server = new Server();
	server->configure(scfg, address);
	
	addAgent(server);
	return server;
}

void PolySrv::configure() {
	// in general, server-side is better off with a timeout; change defaults
	if (TheOpts.theIdleTout == Time())
		TheOpts.theIdleTout.set(Time::Sec(5*60));
	PolyApp::configure();
	Server::FtpFarm(new FtpSrvXactFarm);
	Server::HttpFarm(new HttpSrvXactFarm);
	StatIntvl::ActiveCat(lgcSrvSide);
	StatIntvl::TheReportCat = lgcSrvSide;
}

void PolySrv::getOpts(Array<OptGrp*> &opts) {
	PolyApp::getOpts(opts);
	opts.append(&TheSrvOpts);
}

void PolySrv::logState(OLog &log) {
	PolyApp::logState(log);
}

const String PolySrv::sideName() const {
	return "server";
}

int PolySrv::logCat() const {
	return lgcSrvSide;
}

int main(int argc, char *argv[]) {
	PolySrv app;
	return app.run(argc, argv);
}
