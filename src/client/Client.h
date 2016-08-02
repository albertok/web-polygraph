
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
class PrivCache;
class RobotSym;
class SessionMgr;
class HttpPrinter;
namespace Kerberos { class Mgr; };

// robot message count totals, either lifetime or for a busy period
class MessageCounts {
	public:
		MessageCounts(): requests(0), responses(0), authed(0) {}

		void clear() { requests = responses = authed = 0; }
		ostream &print(ostream &os) const;

		uint64_t requests; // request headers formatted to be sent
		uint64_t responses; // final response headers received
		uint64_t authed; // authenticated transactions XXX
};
extern bool operator ==(const MessageCounts &m1, const MessageCounts &m2); 
inline bool operator !=(const MessageCounts &m1, const MessageCounts &m2) { return !(m1 == m2); }

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
		void noteGssContext(CltXact &x, const bool success);
		void noteXactDone(CltXact *x);
		void noteRequestSent() { ++thePeriodMessages.requests; ++theLifetimeMessages.requests; }
		void noteResponseReceived() { ++thePeriodMessages.responses; ++theLifetimeMessages.responses; }
		void noteAuthed() { ++thePeriodMessages.authed; ++theLifetimeMessages.authed; }

		void selectTarget(ObjId &oid); // embedded objs and redirects need it
		void selectScheme(ObjId &oid);

		const String &credentials() const { return theCredentials; }
		bool usesPassiveFtp() const { return usePassiveFtp; }
		PrivCache *privCache() { return thePrivCache; }
		const MessageCounts &periodMessages() const { return thePeriodMessages; }
		const MessageCounts &lifetimeMessages() const { return theLifetimeMessages; }

		ContentCfg *selectReqContent(const ObjId &oid, ObjId &reqOid);

		virtual void becomeBusy();
		virtual void continueSession();
		virtual void becomeIdle();

		virtual void missWorkingSet();

		virtual int logCat() const;

		virtual void describe(ostream &os) const;

		// warning: a cc xaction may change result in another xaction lifetime
		HttpAuthScheme proxyAuthSchemeNow(const ObjId &oid) const;
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
		RangeCfg::RangesInfo makeRangeSet(HttpPrinter &hp, const ObjId &oid, ContentCfg &contentCfg) const;

		PortMgr *portMgr();
		bool usingKerberos() const { return theKerberosMgr; }

		CltXact *dequeSuspXact(CltXact *x = 0);

		void noteKerberosFailure();

	protected:
		// Counts events in a given category; for basic reporting needs
		struct EventCounter {
			explicit EventCounter(const char *cat): what(cat), count(0) {}
			const char *what; // what happened
			unsigned int count; // how many times it happened
		};

		bool shouldRetry(const CltXact *x) const;

		bool tryLaunch();
		bool tryLaunch(CltXact *x);
		bool authAndLaunch(CltXact *x);
		bool suspendXact(CltXact *x, EventCounter &stats);
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

		virtual void noteLogEvent(BcastChannel *ch, OLog &log);
		virtual void noteMsgStrEvent(BcastChannel *ch, const char *msg);

		struct ReqId {
			UniqId id;
			int lastName;
		};
		void configureReqIds(Array<ReqId> &ids, const Array<ContentCfg*> &cfgs);

		bool setNextHopAddr(CltXact *x) const;
		void setNextHopIp(CltXact *x, const NetAddr &addr) const;

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
		static XactFarm<CltXact> *TheFtpXacts;
		static XactFarm<CltXact> *TheHttpXacts;
		static ObjFarm<IcpCltXact> TheIcpXacts;

		PrivCache *thePrivCache;        // private cache for this agent

		Queue<CltXact, &CltXact::waitingXacts> theLaunchDebts; // postponed requests

		CltCfg *theCfg;         // client configuration (maybe shared)
		CltConnMgr *theConnMgr; // one for all servers
		DnsMgr *theDnsMgr;      // DNS client, cache, etc.
		Kerberos::Mgr *theKerberosMgr; // Kerberos authentication
		SessionMgr *theSessionMgr; // manages busy/idle periods

		NetAddr theSocksProxyAddr;
		NetAddr theFtpProxyAddr;
		NetAddr theHttpProxyAddr;
		SslCtx *theProxySslCtx; // context for SSL-to-proxy connections

		// XXX: group all per-session objects into one struct and reset
		String theCredentials; // per-session username and password
		Memberships theMemberships; // per-session user membership

		int theCcXactLvl;      // outstanding concurrent xactions level
		int theExtraLaunchLvl; // uncompensated out-of-order launches

		MessageCounts theLifetimeMessages; // never reset
		MessageCounts thePeriodMessages; // reset every time we become busy

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
		bool isRunning; // between start() and stop()
		bool isIdle;
};

#endif
