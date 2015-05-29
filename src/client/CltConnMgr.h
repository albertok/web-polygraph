
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CLTCONNMGR_H
#define POLYGRAPH__CLIENT_CLTCONNMGR_H

#include "xstd/NetAddr.h"
#include "xstd/Queue.h"
#include "client/ConnHash.h"
#include "runtime/ConnMgr.h"

class Client;
class ObjId;
class RndDistr;
class PortMgr;
class SslSession;

class CltConnMgr: public ConnMgr {
	public:
		CltConnMgr();
		virtual ~CltConnMgr();

		void configure(const SockOpt &anOpt, const Client *aClient, const int srvCnt);
		virtual void configureSsl(SslCtx *aCtx, const SslWrap *wrap);
		void portMgr(PortMgr *aPortMgr);
		PortMgr *portMgr();

		bool atHardConnLimit() const; 
		Connection *get(const ObjId &oid, const NetAddr &hopAddr, const NetAddr &tcpHopAddr, ProtoIntvlPtr protoStat);

		void closeAllIdle(); // close all idle connections

		virtual void noteReadReady(int fd);

		virtual bool hasCredentials() const;
		virtual bool credentialsFor(const Connection &, UserCred &) const;

	protected:
		Connection *open(const NetAddr &hopAddr, const NetAddr &tcpHopAddr, ProtoIntvlPtr protoStat, bool needsSsl);
		Connection *open(const NetAddr &hopAddr, ProtoIntvlPtr protoStat, bool needsSsl);
		virtual void putIdle(Connection *conn);
		virtual void delIdle(Connection *conn);
		virtual void closePrep(Connection *conn);

		bool findIdle(const NetAddr &hopAddr, const NetAddr &tunnelAddr, ConnHashPos &pos);
		bool findIdleSubst(const NetAddr &addr, const NetAddr &tunnelAddr, ConnHashPos &pos);
		bool needSsl(const ObjId &oid, const NetAddr &hopAddr, const NetAddr &tunnelAddr, bool &needTunnel) const;
		bool needMoreSslSessions() const;

	protected:
		const Client *theClient;
		ConnHash theIdleHash;
		Queue<Connection, &Connection::idleConnections> theIdleQueue;
		PortMgr *thePortMgr;

		RndDistr *thePipeDepth;

		Array<SslSession*> theSslSessionCache;
		double theSslResumpProb;  // how often to try to resume a session
		int theSslCacheLimit;     // desired maximum number of cached sessions

		double theMinNewConnProb; // try limit #open() calls using substitutes
		int theConnLvlLmt;        // limit for #opened connections
};

#endif
