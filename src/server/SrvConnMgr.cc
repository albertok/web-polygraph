
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Clock.h"
#include "xstd/Ssl.h"
#include "base/UniqId.h"
#include "base/polyLogCats.h"
#include "runtime/SslWrap.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "runtime/globals.h"
#include "server/SrvConnMgr.h"


SrvConnMgr::SrvConnMgr(User *aUser): protoStat(0), theUser(aUser),
	theAcceptLmt(-1) {
	Assert(theUser);
}

void SrvConnMgr::acceptLmt(int aLmt) {
	theAcceptLmt = aLmt;
}

void SrvConnMgr::configureSsl(SslCtx *aCtx, const SslWrap *wrap) {
	ConnMgr::configureSsl(aCtx, wrap);
	if (!theSslCtx)
		return;

	const int cacheSize = wrap->sessionCacheSize();
	if (wrap->resumpProb() > 0 && cacheSize != 0) {
		aCtx->sessionCacheMode(SSL_SESS_CACHE_SERVER);
		aCtx->sessionId(theUser->id().str());
		if (cacheSize > 0)
			aCtx->sessionCacheSize(cacheSize);
	} else {
		aCtx->sessionCacheMode(SSL_SESS_CACHE_OFF);
	}
}

// return false iff fatal error
bool SrvConnMgr::accept(Socket &s) {
	static const Time acceptTimeLmt = Time::Msec(10);
	const Time startTime = TheClock;
	// note: negative accept limit means no limit
	for (int i = theAcceptLmt; i; --i) {
		bool fatal = false;
		Connection *conn = TheConnFarm.get();
		conn->logCat(lgcSrvSide);
		conn->protoStat = protoStat;
		if (theSslCtx)
			conn->useSsl(theSslCtx, 0);
		if (!conn->accept(s, theSockOpt, fatal)) {
			// note: no close() for an un-opened conn
			TheConnFarm.put(conn);
			return !fatal;
		}
		opened(conn);

		if (theUser->writeFirst())
			conn->theWr.start(this);
		else
			conn->theRd.start(this);

		// occasionally, check if we have been acceting for too long
		if (i % 10 == 0) {
			Clock::Update(false);
			if (TheClock - startTime > acceptTimeLmt)
				break;
		}
	}
	return true;
}

// this gets called for an idle HTTP/FTP conn and on the freshly accepted HTTP conn
void SrvConnMgr::noteReadReady(int fd) {
	Connection *conn = TheConnIdx[fd];
	Assert(conn);
	TheConnIdx.check(conn);

	if (conn->closing()) {
		ConnMgr::noteReadReady(fd);
	} else {
		conn->theRd.stop(this);
		TheFileScanner->clearTimeout(fd);

		conn->read(); // we do the first read, not a xaction
		if (!conn->bad()) {
			if (!conn->exhausted()) {
				if (conn->useCnt())
					endedIdle(conn);
				conn->startUse();
				theUser->noteConnReady(conn);
				return;
			}

			if (!conn->useCnt())
				ReportError2(errNoHdrClose, lgcSrvSide);
		}

		if (conn->useCnt())
			closeIdle(conn, ConnCloseStat::ckIdleForeign);
		else
			closeBusy(conn);
	}
}

// this gets called for (closing HTTP?) and the freshly accepted FTP conn
void SrvConnMgr::noteWriteReady(int fd) {
	Connection *conn = TheConnIdx[fd];
	Assert(conn);
	TheConnIdx.check(conn);

	if (conn->closing()) {
		ConnMgr::noteWriteReady(fd);
	} else {
		conn->theWr.stop(this);
		TheFileScanner->clearTimeout(fd);
		conn->startUse();
		theUser->noteConnReady(conn);
	}
}

void SrvConnMgr::putIdle(Connection *conn) {
	Assert(!conn->theRd.theReserv && !conn->theWr.theReserv);
	// there may be more requests waiting
	if (conn->theRdBuf.contSize()) {
		conn->startUse();
		theUser->noteConnReady(conn);
	} else {
		ConnMgr::putIdle(conn);
	}
}
