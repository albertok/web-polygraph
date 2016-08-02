
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__SERVER_SERVER_H
#define POLYGRAPH__SERVER_SERVER_H

#include "xstd/FileScanner.h"
#include "runtime/Agent.h"
#include "runtime/XactFarm.h"
#include "runtime/BcastRcver.h"
#include "server/SrvCfg.h"
#include "server/SrvXact.h"
#include "server/SrvConnMgr.h"


class PopModel;
class ServerSym;


class Server: public Agent, public FileScanner::User, public SrvConnMgr::User, public BcastRcver {
	public:
		enum ReqTypes { rptBasic = 1, rpt302Found };

	public:
		static void LogState(OLog &log);
		static void FtpFarm(XactFarm<SrvXact> *aFarm);
		static void HttpFarm(XactFarm<SrvXact> *aFarm);

	public:
		Server();
		~Server();

		void configure(const ServerSym *cfg, const NetAddr &aHost);
		virtual void describe(ostream &os) const;

		virtual void start();
		virtual void stop();

		// xactions need access to this
		int hostIdx() const { return theHostIdx; }
		virtual SrvCfg *cfg() { return theCfg; }
		PopModel *popModel();

		void selectRepType(ObjId &oid);

		void noteXactDone(SrvXact *x);

		virtual void noteReadReady(int fd);
		virtual void noteConnReady(Connection *conn);

		virtual int logCat() const;
		virtual const UniqId &id() const;
		virtual bool writeFirst() const;

		PortMgr *portMgr();

	protected:
		Agent::Protocol protocol() const;

		void configureContents(const ServerSym *cfg);
		void startXact(Connection *conn);
		void deaf();

		virtual void noteLogEvent(BcastChannel *ch, OLog &log);

	protected:
		static XactFarm<SrvXact> *TheFtpXacts;
		static XactFarm<SrvXact> *TheHttpXacts;

		SrvCfg *theCfg;           // server configuration (maybe shared)

		PortMgr *thePortMgr;      // port manager for active FTP connections
		SrvConnMgr *theConnMgr;
		Socket theSock;
		SockOpt theListenOpt;

		int theHostIdx;           // index into the HostMap
		int theReqCount;          // number of accepted requests

		FileScanReserv theReserv;
};

#endif
