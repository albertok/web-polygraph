
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Rnd.h"
#include "xstd/rndDistrs.h"
#include "xstd/TblDistr.h"
#include "xstd/gadgets.h"
#include "base/RndPermut.h"
#include "base/ObjId.h"
#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "base/rndDistrStat.h"
#include "runtime/Connection.h"
#include "runtime/HostMap.h"
#include "runtime/AddrMap.h"
#include "runtime/PortMgr.h"
#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/PopModel.h"
#include "runtime/StatPhase.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "runtime/globals.h"
#include "runtime/SslWrap.h"
#include "runtime/SslWraps.h"
#include "pgl/ServerSym.h"
#include "pgl/SocketSym.h"
#include "pgl/PopModelSym.h"
#include "server/SrvCfg.h"
#include "server/SrvConnMgr.h"
#include "server/SrvOpts.h"
#include "server/Server.h"


XactFarm<SrvXact> *Server::TheFtpXacts;
XactFarm<SrvXact> *Server::TheHttpXacts;


void Server::FtpFarm(XactFarm<SrvXact> *aFarm) {
	Assert(!TheFtpXacts && aFarm);
	TheFtpXacts = aFarm;
}

void Server::HttpFarm(XactFarm<SrvXact> *aFarm) {
	Assert(!TheHttpXacts && aFarm);
	TheHttpXacts = aFarm;
}

void Server::LogState(OLog &) {
}

Server::Server(): theCfg(0), theHostIdx(-1), theReqCount(0) {
	theConnMgr = new SrvConnMgr(this); // for now

	theChannels.append(TheLogCfgChannel);
	theChannels.append(TheLogStateChannel);
	startListen();
}

Server::~Server() {
	stop();
	delete theConnMgr;
}

void Server::configure(const ServerSym *cfg, const NetAddr &aHost) {
	Assert(theConnMgr);

	theCfg = TheSrvSharedCfgs.getConfig(cfg);

	Agent::configure(cfg, aHost);
	if (theHost.port() < 0) {
		cerr << cfg->loc() << "no port specified for the server `"
			<< theKind << "' running on " << theHost << endl;
		exit(-3);
	}

	// XXX: cannot Assert: pxy server does not have a HostMap entry
	if (!TheHostMap->find(aHost, theHostIdx))
		theHostIdx = -1;

	// setup listen socket (XXX: should be moved to start(), use Comment)
	theSock = makeListenSocket(theHost);
	if (!theSock) {
		cerr << "error: the server `" << theKind << "' cannot listen on " << theHost
			<< "; " << Error::Last() << endl;
		exit(-3);
	}

	int acceptLmt = -1;
	cfg->acceptLmt(acceptLmt);
	theConnMgr->configure(theSockOpt, cfg->pconnUseLmt());
	theConnMgr->acceptLmt(acceptLmt);
	theConnMgr->idleTimeout(cfg->idlePconnTimeout());

	const SslWrap *sslWrap = 0;
	if (theCfg->sslActive(protocol()) &&
		theCfg->selectSslWrap(sslWrap)) { // sticky selection
		theSslCtx = sslWrap->makeServerCtx(theHost);
		theConnMgr->configureSsl(theSslCtx, sslWrap);
	}

	if (!TheAddrMap->has(theHost)) // PolyApp adds only if findAddr() fails
		TheAddrMap->add(theHost);

	isCookieSender = theCfg->selectCookieSenderStatus();
	if (isCookieSender && (!theCfg->theCookieSizes || !theCfg->theCookieCounts)) {
		cerr << cfg->loc() << "error: cookie-sending server does not have " <<
			"cookie size or count distributions configured" << endl;
		FatalError2(errOther, lgcSrvSide);
	}

	thePortMgr = PortMgr::Get(theHost);

	TheFtpXacts->limit(1024); // magic, no good way to estimate
	TheHttpXacts->limit(1024); // magic, no good way to estimate
}

// TODO: should we just cache protocol value instead?
Agent::Protocol Server::protocol() const {
	if (const HostCfg *host = TheHostMap->at(theHostIdx))
		return host->theProtocol;
	else
		return pUnknown;
}

void Server::start() {
	Assert(theSock);
	Assert(theConnMgr);
	theConnMgr->protoStat = (protocol() == pFTP) ? &StatIntvlRec::theFtpStat : 0;
	Agent::start();
	theReserv = TheFileScanner->setFD(theSock.fd(), dirRead, this);
}

void Server::stop() {
	deaf();
	Agent::stop();
}

PopModel *Server::popModel() {
	return theCfg->thePopModel;
}

void Server::selectRepType(ObjId &oid) {
	// XXX: server side does not support global xaction counting yet
	// and hence cannot suuply all the parameters to tf.current below
	static RndGen rng;
	const TransFactor &tf = TheStatPhaseMgr->specialMsgFactor();
	const int repType = rng.event(tf.current()) ?
		(int)theCfg->theRepTypeSel->trial() : rptBasic;
	oid.repToRedir(repType == rpt302Found);
}

void Server::noteXactDone(SrvXact *x) {
	Assert(x);
	theConnMgr->put(x->conn());
	switch (protocol()) {
		case Agent::pFTP:
			TheFtpXacts->put(x);
			break;
		case Agent::pHTTP:
			TheHttpXacts->put(x);
			break;
		default:
			Assert(false);
	}
}

void Server::noteReadReady(int) {
	if (!theConnMgr->accept(theSock)) {	// fatal error
		deaf();
		FatalError2(errServerGone, lgcSrvSide);
	}
}

void Server::noteConnReady(Connection *conn) {
	startXact(conn); // that's it?!
}

void Server::noteLogEvent(BcastChannel *ch, OLog &log) {
	if (ch == TheLogCfgChannel) {
		log << bege(lgSrvCfg, lgcSrvSide);
		// XXX: implement (SrvCfgRec)
		log << ende;
	} else
	if (ch == TheLogStateChannel) {
		log << bege(lgSrvState, lgcSrvSide);
		log << theSeqvId;
		log << ende;
	}
}

int Server::logCat() const {
	return lgcSrvSide;
}

// SrvConnMgr::User needs this
const UniqId &Server::id() const {
	return Agent::id();
}

bool Server::writeFirst() const {
	return protocol() == Agent::pFTP;
}

void Server::deaf() {
	if (theReserv)
		TheFileScanner->clearRes(theReserv);

	if (theSock) {
		Comment(2) << "server " << theHost 
			<< " is closing listen socket " << theSock.fd() 
			<< " after " << theReqCount << " xactions"
			<< endc;
		theSock.close();
	}
}

void Server::startXact(Connection *conn) {
	theReqCount++;
	SrvXact *x = 0;
	switch (protocol()) {
		case Agent::pFTP:
			x = TheFtpXacts->get();
			break;
		case Agent::pHTTP:
			x = TheHttpXacts->get();
			break;
		default:
			Assert(false);
	}
	x->exec(this, conn,
		theThinkDistr ? Time::Secd(theThinkDistr->trial()) : Time());
}

PortMgr *Server::portMgr() {
	return thePortMgr;
}

void Server::describe(ostream &os) const {
	if (theKind)
		os << theKind;
	os << '#' << seqvId() << ' ';

	const char *scheme = 0;
	switch (protocol()) {
		case Agent::pHTTP:
			scheme = "http://";
			break;
		case Agent::pFTP:
			scheme = "ftp://";
			break;
		default:
			scheme = "";
	}

	os << scheme << theHost << "/w" << theId << ' ';

	if (protocol() == Agent::pHTTP) {
		if (theHttpVersion.known())
			os << " HTTP/" << theHttpVersion.vMajor() << '.' << theHttpVersion.vMinor();
		if (theSslCtx)
			os << " SSL";
	}
}
