
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CLIENT_H
#define POLYGRAPH__CLIENT_CLIENT_H

#include <map>

#include "client/CltBehaviorCfg.h"
#include "client/CltCfg.h"
#include "client/CltXact.h"
#include "client/MembershipMap.h"
#include "runtime/httpHdrs.h"
#include "runtime/Agent.h"
#include "runtime/BcastRcver.h"
#include "runtime/XactFarm.h"
#include "xstd/AlarmClock.h"

class CltConnMgr;
class ContentCfg;
class DistrPoint;
class DnsMgr;
class IcpClient;
class IcpCltXact;
class ObjId;
class PortMgr;
class PrivCache;
class RobotSym;
class SessionMgr;

class Client: public Agent, public AlarmUser, public BcastRcver {
	public:
		enum CltReqTypes { rqtBasic = 1, rqtIms200, rqtIms304, rqtReload, rqtRange, rqtUpload };
		enum CltReqMethods { rqmGet = 1, rqmHead, rqmPost, rqmPut };

	public:
		static void FtpFarm(XactFarm<CltXact> *aFarm);
		static void HttpFarm(XactFarm<CltXact> *aFarm);
		static void LogState(OLog &log);

	public:
		Client();
		virtual ~Client();

		virtual void configure(const RobotSym *cfg, const NetAddr &aHost);
		virtual void start();
		virtual void stop();

		// called by the Cache on a miss
		CltXact *fetch(const ObjId &oid, DistrPoint *dp);

		virtual CltCfg *cfg() { return theCfg; }
		const CltCfg *cfg() const { return theCfg; }
		CltConnMgr *connMgr() { return theConnMgr; }
		const Memberships &memberships() const { return theMemberships; }

		bool hasCredentials() const;
		unsigned doCookies() const;

		void noteEmbedded(CltXact *x, const ObjId &embedOid);
		void noteRedirect(CltXact *x, const ObjId &destOid);
		void noteAddrLookup(const NetAddr &addr, CltXact *x);
		void noteXactDone(CltXact *x);

		void selectTarget(ObjId &oid); // embedded objs and redirects need it
		void selectScheme(ObjId &oid);

		const String &credentials() const { return theCredentials; }
		bool usesPassiveFtp() const { return usePassiveFtp; }
		PrivCache *privCache() { return thePrivCache; }

		ContentCfg *selectReqContent(const ObjId &oid, ObjId &reqOid);

		virtual void becomeBusy();
		virtual void continueSession();
		virtual void becomeIdle();

		virtual void missWorkingSet();

		virtual int logCat() const;

		virtual void describe(ostream &os) const;

		HttpAuthScheme proxyAuthScheme(const ObjId &oid) const;
		HttpAuthScheme originAuthScheme(const ObjId &oid) const;
		bool credentialsFor(ObjId &, UserCred &) const;
		bool credentialsFor(const Connection &, UserCred &) const;
		void noteProxyAuthReq(CltXact *, HttpAuthScheme);
		void noteOriginAuthReq(CltXact *, HttpAuthScheme);
		void notePeerAsked(IcpCltXact *);
		IcpClient *icpClient();

		const NetAddr &socksProxy() const { return theSocksProxyAddr; }
		const NetAddr &proxy(const ObjId &) const;
		HttpCookies *cookies(const ObjId &) const;
		void absorbCookies(const ObjId &, HttpCookies *&);
		const CltCfg &behavior(const ObjId &oid, const CltBehaviorCfg::Predicate predicate) const;
		RangeCfg::RangesInfo makeRangeSet(ostream &os, const ObjId &oid, ContentCfg &contentCfg) const;

		PortMgr *portMgr();

		CltXact *dequeSuspXact(CltXact *x = 0);

	protected:
		PortMgr *cfgPortMgr();

		bool shouldRetry(const CltXact *x) const;

		bool tryLaunch();
		bool tryLaunch(CltXact *x);
		bool suspendXact(CltXact *x);
		void resumeXact();
		virtual void loneXactFollowup();

		void recycleXact(CltXact *x);

		CltXact *genXact();
		CltXact *genXact(const ObjId &oid, CltXact *cause);
		void genOid(ObjId &oid);
		void selectViserv(ObjId &oid);
		void selectAnyTarget(ObjId &oid, int niamIdx);
		void selectTypedTarget(ObjId &oid, int niamIdx);
		void selectObj(ObjId &oid);
		void selectForeignObj(ObjId &oid);
		void selectContType(ObjId &oid);
		void selectReqType(ObjId &oid);
		void selectReqMethod(ObjId &oid);

		virtual void scheduleLaunch(Time lastLaunch) = 0;
		virtual bool launchCanceled(CltXact *x) = 0; // with body
		bool launchFailed(CltXact *x);

		virtual void noteInfoEvent(BcastChannel *ch, InfoEvent ev);
		virtual void noteLogEvent(BcastChannel *ch, OLog &log);

		struct ReqId {
			UniqId id;
			int lastName;
		};
		void configureReqIds(Array<ReqId> &ids, const Array<ContentCfg*> &cfgs);

		bool setNextHopAddr(CltXact *x) const;

		CltXact *getXact(const ObjId &oid);
		void putXact(CltXact *x);
		Agent::Protocol scheme(const ObjId &oid) const;
		Agent::Protocol transport(const ObjId &oid) const;

		void askPeer(const NetAddr &addr, CltXact *x);

		// URL path:auth scheme
		typedef std::map<String, HttpAuthScheme> UrlPathAuthMap;
		// per robor host specific info
		class HostInfo {
			public:
				HostInfo();
				HostInfo(const HostInfo &);
				~HostInfo();

			private:
				HostInfo &operator =(const HostInfo &);

			public:
				HttpCookies *cookies;
				UrlPathAuthMap *pathAuthMap;
		};
		const HostInfo *hostInfo(const ObjId &oid) const;
		HostInfo &forceHostInfo(const ObjId &oid);

	private:
		bool launch(CltXact *x);
		bool genCredentials(UserCred &cred) const;

	protected:
		static CltSharedCfgs TheSharedCfgs;
		static PtrArray<PortMgr*> ThePortMgrs;  // port managers for all aliases
		static XactFarm<CltXact> *TheFtpXacts;
		static XactFarm<CltXact> *TheHttpXacts;
		static ObjFarm<IcpCltXact> TheIcpXacts;

		PrivCache *thePrivCache;        // private cache for this agent

		Queue<CltXact, &CltXact::waitingXacts> theLaunchDebts; // postponed requests

		CltCfg *theCfg;         // client configuration (maybe shared)
		CltConnMgr *theConnMgr; // one for all servers
		DnsMgr *theDnsMgr;      // DNS client, cache, etc.
		SessionMgr *theSessionMgr; // manages busy/idle periods

		NetAddr theSocksProxyAddr;
		NetAddr theFtpProxyAddr;
		NetAddr theHttpProxyAddr;

		// XXX: group all per-session objects into one struct and reset
		String theCredentials; // per-session username and password
		Memberships theMemberships; // per-session user membership

		int theCcXactLvl;      // outstanding concurrent xactions level
		int theExtraLaunchLvl; // uncompensated out-of-order launches

		unsigned theCookiesKeepLimit; // maximum number of cookies/server to keep

		Array<ReqId> theReqPostIds;
		Array<ReqId> theReqUploadIds;

		IcpClient *theIcpClient;

		Array<HostInfo> theViservsHostInfo; // viserv:host info
		typedef std::map<NetAddr, HostInfo> ForeignHostInfoMap;
		ForeignHostInfoMap theForeignHostInfo; // foreign server:host info

		HttpAuthScheme theFtpProxyAuth; // if set, proxy needs our credentials
		HttpAuthScheme theHttpProxyAuth; // if set, proxy needs our credentials

		bool usePassiveFtp; // if set, uses passive FTP, active otherwise
		bool isIdle;
};

#endif
