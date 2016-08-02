
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"

#include "xstd/Rnd.h"
#include "xstd/Ssl.h"
#include "base/ObjId.h"
#include "base/RndPermut.h"
#include "base/polyLogCats.h"
#include "csm/oid2Url.h"
#include "runtime/globals.h"
#include "runtime/AddrSubsts.h"
#include "runtime/HostMap.h"
#include "runtime/SslWrap.h"
#include "pgl/RobotSym.h"
#include "client/Client.h"
#include "client/CltConnMgr.h"

/* CltConnMgr::SslCache */

CltConnMgr::SslCache::SslCache(): theResumpProb(-1), theLimit(-1) {
}

CltConnMgr::SslCache::~SslCache() {
	while (theSessions.count()) delete theSessions.pop();
}

void CltConnMgr::SslCache::configure(SslCtx *aCtx, const SslWrap *wrap) {
	// we maintain our own cache, if any
	aCtx->sessionCacheMode(SSL_SESS_CACHE_OFF);

	theResumpProb = wrap->resumpProb();
	theLimit = wrap->sessionCacheSize();
}

SslSession *CltConnMgr::SslCache::getSession() {
	SslSession *sess = 0;
	static RndGen rng(LclPermut(rndSslSessionCache));
	if (theSessions.count() && rng.event(theResumpProb))
		sess = theSessions.pop();
	return sess;
}

bool CltConnMgr::SslCache::needMore() const {
	return theResumpProb > 0 && (theLimit < 0 ||
		theSessions.count() < theLimit);
}

void CltConnMgr::SslCache::closePrep(Connection *conn, bool toProxy) {
	const Ssl *ssl = toProxy ? conn->sslActiveProxy() : conn->sslActive();
	if (ssl) {
		SslSession *sess = toProxy ? conn->sslSessionProxy() : conn->sslSession();
		Should(!sess || theResumpProb > 0);

		if (sess && (conn->bad() || !ssl->reusedSession())) {
			// delete sessions of bad connections
			delete sess;
			sess = 0;
			if (toProxy)
				conn->sslSessionForgetProxy();
			else
				conn->sslSessionForget();
		} else
		if (!sess && !conn->bad() && needMore()) {
			// remember sessions of new successful connections
			sess = ssl->refCountedSession();
		}

		if (sess)
			theSessions.push(sess);
	}
}


/* CltConnMgr */

CltConnMgr::CltConnMgr(): theClient(0), thePortMgr(0), thePipeDepth(0),
	theProxySslCtx(0),
	theMinNewConnProb(-1), theConnLvlLmt(-1)  {
}

CltConnMgr::~CltConnMgr() {
	closeAllIdle();
}

void CltConnMgr::configure(const SockOpt &anOpt, const Client *aClient, const int srvCnt) {
	theClient = aClient;
	const RobotSym *const cfg = theClient->cfg()->theRobot;
	ConnMgr::configure(anOpt, cfg->pconnUseLmt());
	cfg->openConnLimit(theConnLvlLmt);
	cfg->minimizeNewConn(theMinNewConnProb);

	if (theMinNewConnProb > 0 && !TheAddrSubsts->count()) {
		cerr << cfg->loc()
			<< "Robot's minimize_new_conn must be used with "
			<< "address substitutes to make sense " << endl << xexit;
	}

	thePipeDepth = cfg->pipelineDepth();

	// determine the maximum number of addresses (for concurrent idle conns)
	const int max = theConnLvlLmt >= 0 ? Min(theConnLvlLmt, srvCnt) : srvCnt;
	theIdleHash.ccAddrMax(max);
}

void CltConnMgr::portMgr(PortMgr *aPortMgr) {
	Assert(aPortMgr && !thePortMgr);
	thePortMgr = aPortMgr;
}

PortMgr *CltConnMgr::portMgr() {
	Assert(thePortMgr);
	return thePortMgr;
}

void CltConnMgr::configureSsl(SslCtx *aCtx, const SslWrap *wrap) {
	ConnMgr::configureSsl(aCtx, wrap);
	if (theSslCtx)
		theSslCacheOrigin.configure(aCtx, wrap);
}

void CltConnMgr::configureProxySsl(SslCtx *aCtx, const SslWrap *wrap) {
	Assert(!theProxySslCtx);
	theProxySslCtx = aCtx;
	if (theProxySslCtx)
		theSslCacheProxy.configure(aCtx, wrap);
}

// note: idle connections can be closed on-demand
bool CltConnMgr::atHardConnLimit() const {
	return theConnLvlLmt >= 0 && (theConnLvl-theIdleQueue.count()) >= theConnLvlLmt;
}

Connection *CltConnMgr::get(const ObjId &oid, const NetAddr &hopAddr, const NetAddr &tcpHopAddr, ProtoIntvlPtr protoStat) {
	static const NetAddr none;
	const NetAddr destAddr(Oid2UrlHost(oid));
	Connection *conn = 0;

	bool needsTunnel = false;
	bool needsSslProxy = false;
	bool needsSsl = needSsl(oid, hopAddr, destAddr, needsTunnel, needsSslProxy);
	const NetAddr &tunnelAddr = needsTunnel ? destAddr : none;
	
	ConnHashPos pos;
	if (findIdle(hopAddr, tunnelAddr, pos)) {
		conn = theIdleHash.delAt(pos);
		theIdleQueue.dequeue(conn);
		conn->theRd.stop(this);
		TheFileScanner->clearTimeout(conn->sock().fd());
		endedIdle(conn);
	} else {
		// check if we should close some idle conn first
		if (theConnLvlLmt >= 0 && theConnLvl >= theConnLvlLmt)
			closeIdle(theIdleQueue.firstOut(), ConnCloseStat::ckIdleLocal);
		conn = open(hopAddr, tcpHopAddr, protoStat, tunnelAddr, needsSsl, needsSslProxy);
	}

	if (conn)
		conn->startUse();

	Assert(theConnLvlLmt < 0 || theConnLvl <= theConnLvlLmt);
	Assert(!conn || (conn->sock() && !conn->bad()));
	return conn;
}

bool CltConnMgr::findIdle(const NetAddr &hopAddr, const NetAddr &tunnelAddr, ConnHashPos &pos) {
	return theIdleHash.find(hopAddr, tunnelAddr, pos) ||
		findIdleSubst(hopAddr, tunnelAddr, pos);
}

void CltConnMgr::closeAllIdle() {
	while (theIdleQueue.count())
		closeIdle(theIdleQueue.firstOut(), ConnCloseStat::ckIdleLocal);
}

Connection *CltConnMgr::open(const NetAddr &hopAddr, const NetAddr &tcpHopAddr, ProtoIntvlPtr protoStat, const NetAddr &tunnelAddr, bool needsSsl, bool needsSslProxy) {
	Connection *conn = TheConnFarm.get();
	conn->logCat(lgcCltSide);
	conn->protoStat = protoStat;

	if (needsSsl)
		conn->useSsl(theSslCtx, theSslCacheOrigin.getSession()); // eventually

	if (needsSslProxy)
		conn->useSslProxy(theProxySslCtx, theSslCacheProxy.getSession());

	// unset for connections that do not need a tunnel
	conn->tunnelEnd(tunnelAddr);

	const NetAddr &socksProxy = hopAddr != tcpHopAddr ?
		tcpHopAddr : NetAddr();
	if (conn->connect(hopAddr, theSockOpt, thePortMgr, socksProxy)) {
		int limit = 1; // no pipeling by default
		if (thePipeDepth)
			limit = (int)MiniMax(1.1, thePipeDepth->trial(), (double)INT_MAX);
		conn->useLevelLimit(limit);

		opened(conn);
		return conn;
	} else {
		// connect failed
		TheConnFarm.put(conn);
		return 0;
	}
}

bool CltConnMgr::needSsl(const ObjId &oid, const NetAddr &hopAddr, const NetAddr &tunnelAddr, bool &needTunnel, bool &needSslProxy) const {
	// need SSL encryption if both source and at least one of the distinations
	// (hop or tunnel) need it
	static const String https("https://");
	bool proxy = hopAddr != tunnelAddr;

	if (proxy && theProxySslCtx && TheHostMap->sslActive(hopAddr))
		needSslProxy = true;

	// without theSslCtx, the source cannot do SSL, except perhaps ssl-to-proxy handled above
	if (!theSslCtx)
		return false;

	if (TheHostMap->sslActive(hopAddr) && !needSslProxy)
		needTunnel = false; // the next hop does SSL
	else
	if (oid.secure())
		needTunnel = proxy; // hop does not do SSL, final does
	else
		return false;      // neither hop nor final do SSL

	return true;
}


// called on an idle connection closed by the other side
void CltConnMgr::noteReadReady(int fd) {
	Connection *conn = TheConnIdx[fd];
	Assert(conn && conn->sock() && !conn->bad());

	if (conn->closing()) {
		ConnMgr::noteReadReady(fd);
	} else {
		// delete connection from the idle queue
		closeIdle(conn, ConnCloseStat::ckIdleForeign);
	}
}

bool CltConnMgr::findIdleSubst(const NetAddr &hopAddr, const NetAddr &tunnelAddr, ConnHashPos &pos) {
	static RndGen rng;
	if (!rng.event(theMinNewConnProb))
		return false;

	Assert(TheAddrSubsts);
	for (AddrSubsts::Iterator i = TheAddrSubsts->iterator(hopAddr); i; ++i) {
		if (theIdleHash.find(i.addr(), tunnelAddr, pos))
			return true;
	}
	return false;
}

void CltConnMgr::putIdle(Connection *conn) {
	Assert(conn && conn->sock() && !conn->bad());
	theIdleQueue.enqueue(conn);
	theIdleHash.add(conn);
	ConnMgr::putIdle(conn);
}

void CltConnMgr::delIdle(Connection *conn) {
	Assert(conn);
	theIdleQueue.dequeue(conn);
	theIdleHash.del(conn);
	conn->theRd.stop(this);
	TheFileScanner->clearTimeout(conn->sock().fd());
}

void CltConnMgr::closePrep(Connection *conn) {
	theSslCacheOrigin.closePrep(conn, false);
	theSslCacheProxy.closePrep(conn, true);
	ConnMgr::closePrep(conn);
}

bool CltConnMgr::hasCredentials() const {
	return theClient && theClient->hasCredentials();
}

bool CltConnMgr::credentialsFor(const Connection &conn, UserCred &cred) const {
	return theClient && theClient->credentialsFor(conn, cred);
}
