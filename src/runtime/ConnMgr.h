
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_CONNMGR_H
#define POLYGRAPH__RUNTIME_CONNMGR_H

#include "xstd/FileScanner.h"
#include "runtime/Farm.h"
#include "runtime/Connection.h"
#include "runtime/ConnIdx.h"

class RndDistr;
class SslCtx;
class SslWrap;
class UserCred;

// interface for managing agent connections
class ConnMgr: public FileScanner::User {
	protected:
		typedef ConnCloseStat::CloseKind CloseKind;

	public:
		ConnMgr();

		void configure(const SockOpt &anOpt, RndDistr *aPconnUseLmt);
		virtual void configureSsl(SslCtx *aCtx, const SslWrap *wrap);
		void idleTimeout(Time aTout);

		// return used connection
		void put(Connection *conn);

		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);
		virtual void noteTimeout(int fd, Time tout);

		virtual bool hasCredentials() const { return false; }
		virtual bool credentialsFor(const Connection &, UserCred &) const { return false; }

	protected:
		virtual void putIdle(Connection *conn);
		virtual void delIdle(Connection *) {}
		void endedIdle(Connection *conn);

		void opened(Connection *conn);
		void setUseLimit(Connection *conn);
		void closeIdle(Connection *conn, CloseKind ck);
		void closeBusy(Connection *conn);

		// do not call directly; call closeIdle/closeBusy instead
		void closeBeg(Connection *conn);
		void closeCont(Connection *conn);
		void closeEnd(Connection *conn);

		// called by closeBeg() before closing starts
		virtual void closePrep(Connection *conn);

	protected:
		static ObjFarm<Connection> TheConnFarm;
		static ConnIdx TheConnIdx; // shared fd -> conn map
	
		SockOpt theSockOpt;
		RndDistr *thePconnUseLmt;
		Time theIdleTimeout;

		const SslCtx *theSslCtx; // owner requests encryption

		int theConnLvl;       // all opened connections
		int theIdleConnLvl;   // idle connections
};

#endif
