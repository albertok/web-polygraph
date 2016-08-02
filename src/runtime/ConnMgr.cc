
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Rnd.h"
#include "xstd/gadgets.h"
#include "base/ConnCloseStat.h"
#include "runtime/ConnIdx.h"
#include "runtime/ConnMgr.h"
#include "runtime/ErrorMgr.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "runtime/globals.h"



ObjFarm<Connection> ConnMgr::TheConnFarm;
ConnIdx ConnMgr::TheConnIdx;


ConnMgr::ConnMgr(): thePconnUseLmt(0), theSslCtx(0), theConnLvl(0) {
	// init static arrays (could init more than once, but that's ok)
	TheConnFarm.limit(Socket::TheMaxLevel+1);
	TheConnIdx.fdLimit(Socket::TheMaxLevel+1);
}

void ConnMgr::configure(const SockOpt &anOpt, RndDistr *aPconnUseLmt) {
	Assert(!thePconnUseLmt);
	thePconnUseLmt = aPconnUseLmt;
	theSockOpt = anOpt;
}

void ConnMgr::configureSsl(SslCtx *aCtx, const SslWrap *) {
	Assert(!theSslCtx);
	theSslCtx = aCtx;
}

void ConnMgr::idleTimeout(Time aTout) {
	theIdleTimeout = aTout;
}

void ConnMgr::put(Connection *conn) {
	conn->finishUse();

	// ignore if somebody is still using this connection
	if (conn->inUse())
		return;

	TheConnIdx.check(conn);

	// see if we want to keep this connection open
	if (conn->reusable())
		putIdle(conn); // may close due to external limits
	else
		closeBusy(conn);
}

// adds freshly opened connection
void ConnMgr::opened(Connection *conn) {
	conn->mgr(this);
	TheConnIdx.add(conn);
	setUseLimit(conn);
	theConnLvl++;
}

void ConnMgr::setUseLimit(Connection *conn) {
	int limit = 1; // no pconns by default
	if (thePconnUseLmt)
		limit = Max(1, (int)thePconnUseLmt->trial());
	conn->useCountLimit(limit);
}

void ConnMgr::putIdle(Connection *conn) {
	// start waiting for a foreign close
	if (theIdleTimeout >= 0)
		conn->theRd.start(this, theIdleTimeout);
	else
		conn->theRd.start(this);
	TheFileScanner->setPriority(conn->sock().fd(), fsupBestEffort);
	Broadcast(TheConnIdleBegChannel, conn);
}

void ConnMgr::endedIdle(Connection *conn) {
	Broadcast(TheConnIdleEndChannel, conn);
}

void ConnMgr::closeIdle(Connection *conn, CloseKind ck) {
	conn->closeKind(ck);
	delIdle(conn); // must go before endedIdle() to sync CltConnMgr::theIdle*
	endedIdle(conn); // may Broadcast(), end test, and trigger closeAllIdle()
	closeBeg(conn);
}

void ConnMgr::closeBusy(Connection *conn) {
	conn->closeKind(ConnCloseStat::ckBusy);
	closeBeg(conn);
}

void ConnMgr::closePrep(Connection *) {
}

// start closing a connection
// may take a while if conenction needs to negotiate closing (e.g., SSL)
void ConnMgr::closeBeg(Connection *conn) {
	closePrep(conn);
	closeCont(conn);
}

void ConnMgr::closeCont(Connection *conn) {
	bool fatal = false;
	if (conn->closeAsync(this, fatal))
		closeEnd(conn);
	else
	if (fatal)
		closeEnd(conn);
	else
		TheFileScanner->setTimeout(conn->fd(), theIdleTimeout); // wait for I/O
}

void ConnMgr::closeEnd(Connection *conn) {
	TheConnIdx.del(conn);
	conn->closeNow();
	TheConnFarm.put(conn);
	theConnLvl--;
}

// this may be called for in-closing connections only
void ConnMgr::noteReadReady(int fd) {
	Connection *conn = TheConnIdx[fd];
	Assert(conn);
	TheConnIdx.check(conn);
	closeCont(conn);
}

// this may be called for in-closing connections only
void ConnMgr::noteWriteReady(int fd) {
	Connection *conn = TheConnIdx[fd];
	Assert(conn);
	TheConnIdx.check(conn);
	closeCont(conn);
}

void ConnMgr::noteTimeout(int fd, Time) {
	Connection *conn = TheConnIdx[fd];
	Assert(conn);
	if (conn->closing()) {
		ReportError2(errSslCloseTout, conn->logCat());
		conn->bad(true);
		closeEnd(conn);
	} else {
		conn->theRd.stop(this);
		closeIdle(conn, ConnCloseStat::ckIdleLocal);
	}
}

