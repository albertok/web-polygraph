
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Ssl.h"
#include "base/ObjId.h"
#include "base/BStream.h"
#include "base/AddrParsers.h"
#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "runtime/AddrMap.h"
#include "runtime/CompoundXactInfo.h"
#include "runtime/HostMap.h"
#include "runtime/HttpCookies.h"
#include "runtime/ObjUniverse.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/PortMgr.h"
#include "runtime/PersistWorkSetMgr.h"
#include "runtime/PopModel.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/polyErrors.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/globals.h"
#include "runtime/SslWrap.h"
#include "runtime/SslWraps.h"
#include "runtime/UserCred.h"
#include "runtime/BcastSender.h"
#include "runtime/ProxyCfg.h"
#include "pgl/ProxySym.h"
#include "csm/BodyIter.h"
#include "csm/ContentSel.h"
#include "csm/ContentCfg.h"
#include "csm/ContentMgr.h"
#include "csm/oid2Url.h"
#include "pgl/RobotSym.h"
#include "dns/DnsMgr.h"
#include "kerberos/Mgr.h"
#include "client/CltConnMgr.h"
#include "client/CltOpts.h"
#include "client/CltXact.h"
#include "client/ForeignWorld.h"
#include "client/IcpCltXact.h"
#include "client/SessionMgr.h"
#include "client/PrivCache.h"
#include "client/CltCfg.h"
#include "client/Client.h"


CltSharedCfgs Client::TheSharedCfgs;
XactFarm<CltXact> *Client::TheFtpXacts = 0;
XactFarm<CltXact> *Client::TheHttpXacts = 0;
ObjFarm<IcpCltXact> Client::TheIcpXacts;


void Client::FtpFarm(XactFarm<CltXact> *aFarm) {
	Assert(!TheFtpXacts && aFarm);
	TheFtpXacts = aFarm;
}

void Client::HttpFarm(XactFarm<CltXact> *aFarm) {
	Assert(!TheHttpXacts && aFarm);
	TheHttpXacts = aFarm;
}

Client::Client(): thePrivCache(0), theCfg(0), 
	theConnMgr(0), theDnsMgr(0), theKerberosMgr(0), theSessionMgr(0),
	theProxySslCtx(0),
	theCcXactLvl(0), theExtraLaunchLvl(0), theCookiesKeepLimit(0),
	theIcpClient(0),
	theFtpProxyAuth(authNone), theHttpProxyAuth(authNone),
	usePassiveFtp(true),
	isRunning(false),
	isIdle(false) {
	theChannels.append(TheLogCfgChannel);
	theChannels.append(TheLogStateChannel);
	theChannels.append(ThePhasesEndChannel);
	startListen();
}

Client::~Client() {
	delete theConnMgr;
	delete theDnsMgr;
	delete theKerberosMgr;
	delete theSessionMgr;
	delete thePrivCache;
}

void Client::configure(const RobotSym *cfg, const NetAddr &aHost) {
	Assert(aHost.port() < 0); // remove later

	theCfg = TheSharedCfgs.getConfig(cfg);

	Agent::configure(cfg, aHost);

	int pcCap = 0;
	if (cfg->privCache(pcCap))
		thePrivCache = new PrivCache(pcCap);

	bool doSocksChaining = false;
	theCfg->selectSocksProxy(theSocksProxyAddr, doSocksChaining); // sticky selection
	if (!theSocksProxyAddr || doSocksChaining) {
		theCfg->selectHttpProxy(theHttpProxyAddr); // sticky selection
		if (!theCfg->selectFtpProxy(theFtpProxyAddr)) // sticky selection
			theFtpProxyAddr = theHttpProxyAddr;
	}
	int srvCnt;
	if (theFtpProxyAddr && theHttpProxyAddr)
		srvCnt = theFtpProxyAddr == theHttpProxyAddr ? 1 : 2;
	else
		srvCnt = theCfg->viservCount();
	theConnMgr = new CltConnMgr;
	theConnMgr->configure(theSockOpt, this, srvCnt);
	theConnMgr->portMgr(PortMgr::Get(theHost));
	theConnMgr->idleTimeout(cfg->idlePconnTimeout());

	const SslWrap *sslWrap = 0;
	if (theCfg->selectSslWrap(sslWrap)) { // sticky selection
		theSslCtx = sslWrap->makeClientCtx(theHost);
		theConnMgr->configureSsl(theSslCtx, sslWrap);
	}

	HostCfg *cfgHttpProxy = 0;
	if (theHttpProxyAddr && (cfgHttpProxy = TheHostMap->find(theHttpProxyAddr))) {
		if (cfgHttpProxy->isSslActive && cfgHttpProxy->theHostsBasedCfg) {
			const ProxySym &proxySym = dynamic_cast<const ProxySym&>
				(cfgHttpProxy->theHostsBasedCfg->cast(ProxySym::TheType));

			ProxyCfg *proxyCfg = TheProxySharedCfgs.getConfig(&proxySym);
			sslWrap = 0;
			if (proxyCfg->selectSslWrap(sslWrap)) {
				theProxySslCtx = sslWrap->makeClientCtx(theHttpProxyAddr);
				theConnMgr->configureProxySsl(theProxySslCtx, sslWrap);
			}
		}
	}

	theDnsMgr = new DnsMgr(this);
	theDnsMgr->configure(cfg->dnsResolver());

	if (const KerberosWrapSym *const s = cfg->kerberosWrap())
		theKerberosMgr = new Kerberos::Mgr(*this, *s);

	if (theCfg->theBusyPeriod) {
		theSessionMgr = new SessionMgr(this);
		theSessionMgr->configure(theCfg);
	}

	if (theCfg->thePeerHttp && !theCfg->thePeerIcp) {
		cerr << cfg->loc() << "the HTTP peer is at " << theCfg->thePeerHttp << ", but where is the ICP peer?" << endl;
		exit(-3);
	}

	isCookieSender = theCfg->selectCookieSenderStatus();
	if (isCookieSender) {
		theCookiesKeepLimit = theCfg->theCookiesKeepLimitSel ?
			theCfg->theCookiesKeepLimitSel->ltrial() : 4;
	}

	configureReqIds(theReqPostIds, theCfg->thePostContents);
	configureReqIds(theReqUploadIds, theCfg->theUploadContents);

	theViservsHostInfo.resize(theCfg->viservLimit());

	theCfg->selectFtpMode(usePassiveFtp); // sticky selection

	Assert(TheFtpXacts);
	TheFtpXacts->limit(1024); // magic, no good way to estimate
	Assert(TheHttpXacts);
	TheHttpXacts->limit(1024); // magic, no good way to estimate
}

void Client::start() {
	Assert(theConnMgr);
	Assert(theDnsMgr);
	ShouldUs(!isRunning);
	isRunning = true;

	Agent::start();

	theDnsMgr->start();

	theCfg->startWarmup(); // warmup plan created if needed

	if (theSessionMgr)
		theSessionMgr->start();
	else
		becomeBusy();
}

void Client::stop() {
	ShouldUs(isRunning);
	isRunning = false;

	if (theSessionMgr)
		theSessionMgr->stop();
	else
	if (!isIdle)
		becomeIdle();

	theDnsMgr->stop();
	Agent::stop();
}

void Client::becomeBusy() {
	isIdle = false;

	thePeriodMessages.clear();

	theMemberships.reset();
	theCfg->selectCredentials(theCredentials);
	theCfg->findMemberships(theCredentials, theMemberships);

	Broadcast(TheSessionBegChannel, this);

	if (theKerberosMgr && hasCredentials())
		theKerberosMgr->becomeBusy();

	scheduleLaunch(TheClock);
}

void Client::continueSession() {
	Broadcast(TheSessionCntChannel, this);
}

void Client::becomeIdle() {
	isIdle = true;

	if (thePrivCache)
		thePrivCache->clear();

	theFtpProxyAuth = theHttpProxyAuth = authNone;

	while (theLaunchDebts.count())
		launchCanceled(dequeSuspXact());
	theExtraLaunchLvl = 0;

	if (theKerberosMgr)
		theKerberosMgr->becomeIdle();

	theConnMgr->closeAllIdle();

	for (int i = 0; i < theViservsHostInfo.count(); ++i) {
		if (HttpCookies *const cookies = theViservsHostInfo[i].cookies)
			cookies->purgeDiscardable();
	}
	for (ForeignHostInfoMap::const_iterator i = theForeignHostInfo.begin();
		i != theForeignHostInfo.end();
		++i) {
		if (HttpCookies *const cookies = i->second.cookies)
			cookies->purgeDiscardable();
	}

	Broadcast(TheSessionEndChannel, this);
}

void Client::describe(ostream &os) const {
	Agent::describe(os);
	if (theSocksProxyAddr || theFtpProxyAddr || theHttpProxyAddr) {
		os << " via ";
		if (theSocksProxyAddr) {
			os << theSocksProxyAddr << " (socks)";
			if (theHttpProxyAddr || theFtpProxyAddr)
				os << ", ";
		}
		if (theHttpProxyAddr) {
			if (theProxySslCtx)
				os << "https://";
			os << theHttpProxyAddr << " (http";
			if (theFtpProxyAddr == theHttpProxyAddr)
				os << " and ftp)";
			else if (theFtpProxyAddr)
				os << ") or ";
			else
				os << ')';
		}
		if (theFtpProxyAddr && theFtpProxyAddr != theHttpProxyAddr)
			os << theFtpProxyAddr << " (ftp)";
	}
}

void Client::noteLogEvent(BcastChannel *ch, OLog &log) {
	if (ch == TheLogCfgChannel) {
		log << bege(lgCltCfg, lgcCltSide);
		// XXX: implement (CltCfgRec)
		log << ende;
	} else
	if (ch == TheLogStateChannel) {
		log << bege(lgCltState, lgcCltSide)
			<< theSeqvId 
			<< ende;
	}
}

void Client::noteMsgStrEvent(BcastChannel *ch, const char *) {
	Assert(ch == ThePhasesEndChannel);
	if (isRunning)
		stop();
}

void Client::noteKerberosFailure() {
	if (theKerberosMgr)
		theKerberosMgr->noteAuthFailure();
}

void Client::noteXactDone(CltXact *x) {
	Assert(x);
	if (!Should(x->conn()))
		return; // XXX: fix me

	const ObjId oid(x->oid());

	// XXX: request type may be different this time
	CltXact *retry = shouldRetry(x) ? genXact(oid, x) : 0;

	if (theSessionMgr)
		theSessionMgr->noteXactDone(x);

	if (!Should(x->conn()))
		return; // XXX: fix me
	if (isIdle)
		x->conn()->lastUse(true); // close all connections if we are idling

	theConnMgr->put(x->conn());
	recycleXact(x);
	theCcXactLvl--;
	Assert(theCcXactLvl >= 0);

	// no xaction should restart when we are idle
	if (isIdle) {
		Should(!retry);
		return;
	}

	if (retry && tryLaunch(retry)) {
		// no lvl increment if we are continuing the same compound transaction
		if (Xaction *cause = retry->cause())
			if (cause->preliminary())
				return;
		++theExtraLaunchLvl;
		return;
	}

	// push waiting xactions forward
	if (theLaunchDebts.count() && !theConnMgr->atHardConnLimit()) {
		resumeXact();
		return;
	}

	if (theCcXactLvl == 0)
		loneXactFollowup();
}

void Client::loneXactFollowup() {
}


CltXact *Client::fetch(const ObjId &oid, DistrPoint *dp) {
	Assert(oid);
	CltXact *x = genXact(oid, 0);
	x->cacheDistrPoint(dp);
	if (tryLaunch(x))
		return x;
	return 0;
}

// must not be called directly but rather through tryLaunch
bool Client::launch(CltXact *x) {
	Assert(x);

	Connection *conn = x->conn(); // non-zero if pipelining

	if (!conn) {
		Assert(!theConnMgr->atHardConnLimit());

		const NetAddr &nextHighHop = x->nextHighHop();
		Assert(nextHighHop);

		const NetAddr &nextTcpHop = x->nextTcpHop();
		Assert(nextTcpHop);
		Assert(!nextTcpHop.isDomainName());

		const ProtoIntvlPtr protoStat = transport(x->oid()) == pFTP ?
			&StatIntvlRec::theFtpStat : 0;
		conn = theConnMgr->get(x->oid(), nextHighHop, nextTcpHop, protoStat);

		if (!conn && ReportError(errConnectEstb)) {
			Comment(1) << theHost << " failed to connect to "
				<< nextTcpHop << endc;
		}
	}

	if (conn) {
		theCcXactLvl++;
		x->exec(conn);
		return true;
	}

	launchFailed(x);
	return false;
}

// tries to generate and launch a transaction
// returns true if the xaction will be launched [eventually]
bool Client::tryLaunch() {
	if (theExtraLaunchLvl) {
		--theExtraLaunchLvl;
		return false;
	}
	
	return tryLaunch(genXact());
}

// will launch the xaction (if possible) or postpone it (if not)
// returns true if the xaction will be launched [eventually]
bool Client::tryLaunch(CltXact *x) {
	Assert(x);

	if (isIdle)
		return launchCanceled(x);

	if (!x->nextHighHop()) {

		// should we ask peers for the best server?
		// XXX: remove askedPeer; the x->nextHighHop() should be enough?
		if (theCfg->thePeerIcp && !x->askedPeer()) {
			askPeer(theCfg->thePeerIcp, x);
			return true;
		}

		// next hop address should be known at this time 
		// (but my not be resolved yet)
		if (!setNextHopAddr(x))
			return false;

		// should we lookup the next hop address?
		NetAddr addr;
		switch (theDnsMgr->instantLookup(x->nextTcpHop(), addr)) {
			case DnsMgr::dnsAlreadyAnIp:
				break; // just continue with the transaction
			case DnsMgr::dnsCacheHit:
				setNextHopIp(x, addr);
				break; // and now continue with the transaction
			case DnsMgr::dnsNeedsAsyncLookup:
				// async call unless fails immediately
				if (theDnsMgr->lookup(x->nextTcpHop(), x))
					return true; // wait for Client::noteAddrLookup()
				launchFailed(x);
				return false;
		}
	}

	x->lifeTimeLimit(selectLifetime());

	// check if we should postpone the xaction due to open connection limit
	if (!x->conn() && theConnMgr->atHardConnLimit()) {
		static EventCounter stats("configured robot open connection limit");
		return suspendXact(x, stats);
	}

	return authAndLaunch(x);
}

bool Client::authAndLaunch(CltXact *x) {
	Assert(x);

	x->freezeProxyAuth(); // we have to do it before calling needGssContext()

	// Do this after theConnMgr->atHardConnLimit() check, so that authenticator
	// we get now does not become "stale" while we wait for an HTTP conn slot.
	if (theKerberosMgr && x->needGssContext()) { // Kerberos context needed
		// TODO: support Kerberos CC limits, even though suspending here
		// may make our open_conn_lmt slot unavailable again, requiring
		// authenticator re-generation or making authenticator stale?
		// Also, the lifetime timer may expire while we createGssContext().
		Assert(!theKerberosMgr->atXactLimit());
		if (theKerberosMgr->createGssContext(proxy(x->oid()), *x)) {
			return true; // wait for Client::noteGssContext()
		} else {
			x->reportAuthError(CltXact::aupAssumedProxy, "while creating GSS context");
			launchFailed(x);
			return false;
		}
	}

	return launch(x);
}

bool Client::suspendXact(CltXact *x, EventCounter &stats) {
	if (theCfg->theWaitXactLmt < 0 || theLaunchDebts.count() < theCfg->theWaitXactLmt) {
		if (!stats.count++)
			Comment(5) << "warning: suspending xaction(s) due to " << stats.what << endc;
		theLaunchDebts.enqueue(x);
		Broadcast(TheWaitBegChannel, x);
		return true;
	}

	if (ReportError(errTooManyWaitXact)) {
		Comment(3) << "xactions active: " << theCcXactLvl 
			<< " waiting: " << theLaunchDebts.count() 
			<< " limit: " << theCfg->theWaitXactLmt << endc;
	}
	launchFailed(x);
	return false;
}

void Client::resumeXact() {
	authAndLaunch(dequeSuspXact());
}

// called with nil x unless called by a lifetime-expired xaction
CltXact *Client::dequeSuspXact(CltXact *x) {
	Must(!theLaunchDebts.empty());
	if (!x)
		x = theLaunchDebts.firstOut();
	theLaunchDebts.dequeue(x);
	Broadcast(TheWaitEndChannel, x);
	return x;
}

bool Client::launchCanceled(CltXact *x) {
	Assert(x);
	x->noteAbort();
	recycleXact(x);
	return false;
}

bool Client::launchFailed(CltXact *x) {
	Assert(x);
	if (!isIdle)
		x->countFailure();
	return launchCanceled(x);
}

void Client::recycleXact(CltXact *x) {
	// recycle x and xactions that caused it
	// stop if a xaction still has kids
	while (x && x->finished() && x->childCount() == 0) {

		// If x is the last part alive, end the compound transaction.
		if (const CompoundXactInfo *compound = x->partOf()) {
			if (compound->ccLevel <= 1)
				Broadcast(TheCompoundXactEndChannel, compound);
		}

		CltXact *cause = x->cause();
		if (cause)
			cause->noteChildGone(x);
		putXact(x);
		x = cause;
	}
}

CltXact *Client::genXact(const ObjId &oid, CltXact *cause) {
	CltXact *x = getXact(oid);
	x->oid(oid);
	x->owner(this);
	if (cause) {
		cause->noteChildNew(x);
		x->cause(cause);
	}
	return x;
}


CltXact *Client::genXact() {
	ObjId oid;
	genOid(oid);
	return genXact(oid, 0);
}

void Client::genOid(ObjId &oid) {
	static RndGen rng;
	if (theCfg->theForeignInterestProb > 0 &&
		rng.event(theCfg->theForeignInterestProb))
		selectForeignObj(oid);
	else {
		selectViserv(oid);
		selectTarget(oid);
		selectContType(oid);
		selectObj(oid);
	}
	selectScheme(oid);
	selectReqType(oid);
	selectReqMethod(oid);

	if (theCfg->genUniqUrls)
		oid.world(UniqId::Create()); // changes every time
}

void Client::selectViserv(ObjId &oid) {
	const int viserv = theCfg->selectViserv();
	HostCfg *host = TheHostMap->at(viserv);
	Assert(host);
	Assert(host->theUniverse);
	Assert(host->theServerRep);
	oid.viserv(viserv);
}

void Client::selectTarget(ObjId &oid) {
	const NetAddr &visName = TheHostMap->at(oid.viserv())->theAddr;
	int niamIdx; // name in AddrMap index
	Assert(TheAddrMap->find(visName, niamIdx));
	if (oid.type() < 0)
		selectAnyTarget(oid, niamIdx);
	else
		selectTypedTarget(oid, niamIdx);

	// sanity checks
	const HostCfg *host = TheHostMap->at(oid.target());
	Assert(host);
	Assert(host->theContent);
}

void Client::selectObj(ObjId &oid) {
	static RndGen rng;

	OidGenStat &oidGenStat = TheStatPhaseMgr->oidGenStat();

	ObjUniverse &universe = *TheHostMap->findUniverseAt(oid.viserv());
	const CltCfg &cfg = behavior(oid, &CltBehaviorCfg::haveRecurRatio);
	const bool needRepeat = rng.event(cfg.theRecurRatio*
		TheStatPhaseMgr->recurFactor().current());
	const bool canRepeat = universe.canRepeat(oid.type());
	const bool forceRepeat = canRepeat && !universe.canProduce(oid.type());
	oidGenStat.recordNeed(needRepeat, OidGenStat::intPublic);

	if (canRepeat && (needRepeat || forceRepeat)) {
		const CltCfg &popModelCfg =
			behavior(oid, &CltBehaviorCfg::havePopModel);
		universe.repeat(oid, popModelCfg.thePopModel);
		oidGenStat.recordGen(true, OidGenStat::intPublic);
	} else {
		universe.produce(oid, rng);
		oidGenStat.recordGen(false, OidGenStat::intPublic);
	}
}

void Client::selectContType(ObjId &oid) {
	Assert(oid.type() < 0); // we do not overwrite existing setting
	const HostCfg *hcfg = TheHostMap->at(oid.target());
	Assert(hcfg);
	Assert(hcfg->theContent);
	const ContentCfg &ccfg = hcfg->theContent->getDir();
	oid.type(ccfg.id());
}

void Client::selectScheme(ObjId &oid) {
	oid.scheme(scheme(oid));
	if (const String &url = oid.foreignUrl()) {
		static const String https("https://");
		oid.secure(url.startsWith(https));
	} else {
		const HostCfg *host = TheHostMap->at(oid.target());
		Assert(host);
		oid.secure(host->isSslActive);
	}
}

void Client::selectReqType(ObjId &oid) {
	static RndGen rng;
	int reqType = rqtBasic;
	if (rng.event(TheStatPhaseMgr->specialMsgFactor().current())) {
		const CltCfg &cfg = behavior(oid, &CltBehaviorCfg::haveReqTypes);
		reqType = cfg.theReqTypeSel->ltrial();
	}

	// force basic request type for all FTP requests except upload
	if (oid.scheme() == Agent::pFTP &&
		reqType != rqtBasic &&
		reqType != rqtUpload)
		reqType = rqtBasic;

	oid.ims200(reqType == rqtIms200);
	oid.ims304(reqType == rqtIms304);
	oid.reload(reqType == rqtReload);
	oid.range(reqType == rqtRange);
	oid.upload(reqType == rqtUpload);
	oid.rediredReq(false);
}

void Client::selectReqMethod(ObjId &oid) {
	static RndGen rng;
	int reqMethod = rqmGet;
	if (oid.upload())
		reqMethod = rqmPut;
	else if (oid.scheme() == Agent::pHTTP &&
		rng.event(TheStatPhaseMgr->specialMsgFactor().current())) {
		const CltCfg &cfg =
			behavior(oid, &CltBehaviorCfg::haveReqMethods);
		reqMethod = cfg.theReqMethodSel->ltrial();
	}

	oid.get(reqMethod == rqmGet);
	oid.post(reqMethod == rqmPost);
	oid.head(reqMethod == rqmHead);
	oid.put(reqMethod == rqmPut);
}

// find any target behind a visible name
void Client::selectAnyTarget(ObjId &oid, int niamIdx) {
	const NetAddr &targetAddr = TheAddrMap->selectAddr(niamIdx);
	int targetIdx = -1;
	Assert(TheHostMap->find(targetAddr, targetIdx));
	oid.target(targetIdx);
}

// find a target that has requested oid type
void Client::selectTypedTarget(ObjId &oid, int niamIdx) {
	Assert(oid.type() >= ContType::NormalContentStart());
	for (AddrMapAddrIter i = TheAddrMap->addrIter(niamIdx); i; ++i) {
		int targetIdx = -1;
		Assert(TheHostMap->find(i.addr(), targetIdx));
		const HostCfg *hcfg = TheHostMap->at(targetIdx);
		Assert(hcfg && hcfg->theContent);
		if (hcfg->theContent->hasContType(oid.type())) {
			oid.target(targetIdx);
			return;
		}
	}

	// we failed to find a target that has the right content
	if (ReportError(errUnreachContType)) {
		static const String strUndefined = "undefined";
		const NetAddr &visName = TheHostMap->at(oid.viserv())->theAddr;
		const String kind = TheContentMgr.get(oid.type())->kind() ?
			TheContentMgr.get(oid.type())->kind() : strUndefined;
		Comment << "robot at " << host() << " cannot find content of '"
			<< kind << "' kind on server(s) visible as " << visName << endc;
	}

	selectAnyTarget(oid, niamIdx);
}

void Client::selectForeignObj(ObjId &oid) {
	Assert(theCfg->foreignWorld());
	ForeignWorld &foreignWorld = *theCfg->foreignWorld();

	static RndGen rng;

	OidGenStat &oidGenStat = TheStatPhaseMgr->oidGenStat();

	const bool needRepeat = rng.event(theCfg->theRecurRatio*
		TheStatPhaseMgr->recurFactor().current());
	oidGenStat.recordNeed(needRepeat, OidGenStat::intForeign);

	const bool canRep = foreignWorld.canRepeat();
	const bool canProd = foreignWorld.canProduce();

	if (canRep && (needRepeat || !canProd)) {
		foreignWorld.repeat(oid, theCfg->thePopModel);
		oidGenStat.recordGen(true, OidGenStat::intForeign);
	} else {
		foreignWorld.produce(oid, rng);
		oidGenStat.recordGen(false, OidGenStat::intForeign);
	}
}

bool Client::credentialsFor(ObjId &oid, UserCred &cred) const {
	if (genCredentials(cred)) {
		cred.finalize(oid);
		oid.authCred(true);
		return true;
	}
	return false;
}

bool Client::credentialsFor(const Connection &conn, UserCred &cred) const {
	if (genCredentials(cred)) {
		cred.finalize(conn);
		return true;
	}
	return false;
}

bool Client::genCredentials(UserCred &cred) const {
	cred = UserCred(theCredentials);
	// invalidate some credentials, if configured
	static RndGen rng; // TODO: needs unique initial seed?
	if (cred.image() && rng.event(theCfg->theAuthError))
		cred.invalidate();
	return cred.image();
}

// whether to collect and, hence, send back cached cookies
unsigned Client::doCookies() const {
	return isCookieSender && theCookiesKeepLimit > 0 ?
		theCookiesKeepLimit : 0;
}

bool Client::shouldRetry(const CltXact *x) const {
	// do not retry if we became idle while trying
	if (isIdle)
		return false;

	// count the number of consequitive retries
	int rcount = 0;
	while (x && x->needRetry()) {
		rcount++;
		x = x->cause();
	}

	if (rcount > 10) {
		ReportError(errManyRetries);
		return false;
	}

	return rcount > 0;
}

void Client::noteProxyAuthReq(CltXact *x, HttpAuthScheme authScheme) {
	if (theFtpProxyAddr == theHttpProxyAddr)
		theFtpProxyAuth = theHttpProxyAuth = authScheme;
	else
	if (x->oid().scheme() == Agent::pHTTP)
		theHttpProxyAuth = authScheme;
	else
		theFtpProxyAuth = authScheme;
}

void Client::noteOriginAuthReq(CltXact *x, HttpAuthScheme authScheme) {
	HostInfo &info(forceHostInfo(x->oid()));
	if (!info.pathAuthMap)
		info.pathAuthMap = new UrlPathAuthMap;
	(*info.pathAuthMap)[Oid2AuthPath(x->oid())] = authScheme;
}

void Client::noteRedirect(CltXact *cause, const ObjId &oid) {
	Assert(oid.rediredReq());
	if (tryLaunch(genXact(oid, cause)))
		theExtraLaunchLvl++;
}

void Client::noteEmbedded(CltXact *parent, const ObjId &oid) {
	static RndGen rng;
	if (!rng.event(theCfg->theEmbedRecurRatio))
		return;

	if (thePrivCache && thePrivCache->loadOid(oid))
		return; // hit, no need to request oid

	CltXact *child = genXact(oid, parent);

	child->page(parent->page());

	// simulate cached DNS responses for embedded objects
	if (!oid.foreignUrl() && oid.viserv() == parent->oid().viserv()) {
		child->nextHighHop(parent->nextHighHop());
		child->nextTcpHop(parent->nextTcpHop());
	}

	// pipeline if possible
	if (child->nextHighHop() &&
		child->nextHighHop() == parent->nextHighHop()) {
		if (PipelinedCxm *mgr = parent->getPipeline())
			child->pipeline(mgr);
	}

	if (tryLaunch(child))
		theExtraLaunchLvl++;
}

void Client::askPeer(const NetAddr &addr, CltXact *x) {
	IcpCltXact *q = TheIcpXacts.get();
	q->reason(x);
	q->exec(this, addr);
}

void Client::notePeerAsked(IcpCltXact *q) {
	Assert(q);

	CltXact *x = q->reason(this);
	x->usePeer(q->hit());
	/* pass ICP stats here ... */

	TheIcpXacts.put(q);

	tryLaunch(x);
}

void Client::setNextHopIp(CltXact *x, const NetAddr &ip) const {
	Assert(ip.addrN().known());
	// We only got the IP address. Host name and port should not change.
	const NetAddr addr(ip.addrN(), x->nextTcpHop().port());
	// TODO: use addr.resolve(ip.addrN()) instead if we can rewrite the
	// !nextTcpHop.isDomainName() assertion in Client::launch to hasIp().
	// const NetAddr addr(x->nextTcpHop()); addr.resolve(ip);

	if (x->nextTcpHop() == x->nextHighHop())
		x->nextHighHop(addr);
	x->nextTcpHop(addr);
}

void Client::noteAddrLookup(const NetAddr &addr, CltXact *x) {
	Assert(x);

	// successful lookup?
	if (addr) {
		setNextHopIp(x, addr);
		tryLaunch(x);
	} else {
		launchFailed(x);
	}
}

// TODO: change profile to (CltXact *x, ccache *)
void Client::noteGssContext(CltXact &x, const bool success) {
	if (success) {
		x.noteGssContext();
		tryLaunch(&x);
	} else {
		x.reportAuthError(CltXact::aupAssumedProxy, "while obtaining a Kerberos ticket");
		launchFailed(&x);
	}
}

// select the address for the next hop connection
bool Client::setNextHopAddr(CltXact *x) const {
	if (theSocksProxyAddr)
		x->nextTcpHop(theSocksProxyAddr);

	NetAddr addr;
	if (x->usePeer() && theCfg->thePeerHttp)
		addr = theCfg->thePeerHttp;
	else
	if (proxy(x->oid()))
		addr = proxy(x->oid());
	else
	if (x->oid().foreignUrl()) {
		const char *uri = x->oid().foreignUrl().cstr();
		if (!SkipHostInUri(uri, uri+x->oid().foreignUrl().len(), addr)) {
			if (ReportError(errHostlessForeignUrl))
				Comment << "foreign URL: " << uri << endc;
			return false;
		}
	}
	else
		addr = Oid2UrlHost(x->oid());

	x->nextHighHop(addr);
	return addr;
}

// fast, used by transactions that connect w/o theConnMgr help
PortMgr *Client::portMgr() {
	Assert(theConnMgr);
	return theConnMgr->portMgr();
}

void Client::configureReqIds(Array<ReqId> &ids, const Array<ContentCfg*> &cfgs) {
	ids.stretch(cfgs.count());
	for (int i = 0; i < cfgs.count(); ++i) {
		ReqId reqId;
		reqId.id.create();
		reqId.lastName = 0;
		ids.append(reqId);
        }
}

void Client::missWorkingSet() {
	// should we freeze all private worlds?
}

int Client::logCat() const {
	return lgcCltSide;
}

void Client::LogState(OLog &log) {
	log << bege(lgSrvRepState, lgcCltSide);
	// XXX: remove state logging?
	// OLogStorePtrs(log, ThePubWorlds);
	log << ende;
}

ContentCfg *Client::selectReqContent(const ObjId &oid, ObjId &reqOid) {
	Assert(!reqOid);

	Array<ReqId> *reqIds;
	RndDistr *sel;
	Array<ContentCfg*> *cfgs;
	if (oid.post()) {
		sel = theCfg->thePostContentSel;
		cfgs = &theCfg->thePostContents;
		reqIds = &theReqPostIds;
	} else
	if (oid.put()) {
		sel = theCfg->theUploadContentSel;
		cfgs = &theCfg->theUploadContents;
		reqIds = &theReqUploadIds;
	} else {
		Assert(false);
		return 0;
	}

	Assert(sel);
	const int idx = sel->ltrial();
	ContentCfg &cfg = *(*cfgs)[idx];
	ReqId &reqId = (*reqIds)[idx];

	reqOid.world(reqId.id);
	reqOid.type(cfg.id());

	static RndGen rng;
	if (reqId.lastName > 0 &&
		theCfg->theReqBodyRecurrence > 0 &&
		rng.event(theCfg->theReqBodyRecurrence)) {
		reqOid.name(1 + rng(0, reqId.lastName));
	} else
		reqOid.name(++reqId.lastName);

	return &cfg;
}

// authNone is returned if no authentication has been required by the proxy yet
HttpAuthScheme Client::proxyAuthSchemeNow(const ObjId &oid) const {
	if (oid.scheme() == Agent::pHTTP) {
		return theHttpProxyAuth;
	} else
		return theFtpProxyAuth;
}

// authNone is returned if no authentication has been required by the server yet
HttpAuthScheme Client::originAuthScheme(const ObjId &oid) const {
	HttpAuthScheme authScheme(authNone);
	const HostInfo *const info(hostInfo(oid));
	if (info && info->pathAuthMap) {
		const UrlPathAuthMap::const_iterator i =
			info->pathAuthMap->find(Oid2AuthPath(oid));
		if (i != info->pathAuthMap->end())
			authScheme = i->second;
	}
	return authScheme;
}

IcpClient *Client::icpClient() {
	return theIcpClient;
}

bool Client::hasCredentials() const {
	return theCredentials;
}

CltXact *Client::getXact(const ObjId &oid) {
	return transport(oid) == Agent::pHTTP ?
		TheHttpXacts->get() : TheFtpXacts->get();
}

void Client::putXact(CltXact *x) {
	if (transport(x->oid()) == Agent::pHTTP)
		TheHttpXacts->put(x);
	else		
		TheFtpXacts->put(x);
}

// Protocol scheme of @oid@.
// FTP scheme() does not imply whether an HTTP or FTP or any proxy is used.
// This method is used only once to set scheme for oid.
// Use oid.scheme() instead.
Agent::Protocol Client::scheme(const ObjId &oid) const {
	static const String ftp("ftp://");
	Agent::Protocol protocol;
	if (!oid.foreignUrl()) {
		const HostCfg *h = TheHostMap->at(oid.target());
		Assert(h && h->theProtocol != Agent::pUnknown);
		protocol = h->theProtocol;
	} else
		protocol = oid.foreignUrl().startsWith(ftp) ?
			Agent::pFTP : Agent::pHTTP;

	return protocol;
}

// Real protocol which is used by @oid@.
// FTP protocol() implies no proxy (i.e., direct FTP traffic).
Agent::Protocol Client::transport(const ObjId &oid) const {
	return theFtpProxyAddr ?
		Agent::pHTTP : static_cast< Agent::Protocol >(oid.scheme());
}

const NetAddr &Client::proxy(const ObjId &oid) const {
	if (oid.scheme() == Agent::pHTTP)
		return theHttpProxyAddr;
	else
		return theFtpProxyAddr;
}

HttpCookies *Client::cookies(const ObjId &oid) const {
	const HostInfo *const info(hostInfo(oid));
	return info ? info->cookies : 0;
}

void Client::absorbCookies(const ObjId &oid, HttpCookies *&c) {
	if (!c)
		return;

	const unsigned limit(doCookies());
	Assert(limit);

	HostInfo &info(forceHostInfo(oid));
	if (info.cookies != c) {
		Assert(!info.cookies);
		c->keepLimit(limit);
		info.cookies = c;
	}
	c = 0;
}

const CltCfg &Client::behavior(const ObjId &oid, const CltBehaviorCfg::Predicate predicate) const {
	Assert(oid.type() >= 0);
	const CltBehaviorCfg *const behaviorCfg =
		TheCltBehaviorCfgs.get(oid.type());
	return *(behaviorCfg && (behaviorCfg->*predicate)() ? behaviorCfg :
		theCfg);
}

RangeCfg::RangesInfo Client::makeRangeSet(HttpPrinter &hp, const ObjId &oid, ContentCfg &contentCfg) const {
	const CltCfg &cfg = behavior(oid, &CltBehaviorCfg::haveRanges);
	return cfg.makeRangeSet(hp, oid, contentCfg);
}

const Client::HostInfo *Client::hostInfo(const ObjId &oid) const {
	const HostInfo *hostInfo(0);
	if (oid.foreignUrl()) {
		const ForeignHostInfoMap::const_iterator i =
			theForeignHostInfo.find(Oid2UrlHost(oid));
		if (i != theForeignHostInfo.end())
			hostInfo = &i->second;
	} else
		hostInfo = &theViservsHostInfo[oid.viserv()];
	return hostInfo;
}

Client::HostInfo &Client::forceHostInfo(const ObjId &oid) {
	return oid.foreignUrl() ?
		theForeignHostInfo[Oid2UrlHost(oid)] :
		theViservsHostInfo[oid.viserv()];
}

/* HostInfo */

Client::HostInfo::HostInfo():
	cookies(0), pathAuthMap(0) {
}

Client::HostInfo::HostInfo(const HostInfo &info)
{
	if (info.cookies)
		cookies = new HttpCookies(*info.cookies);
	else
		cookies = 0;
	if (info.pathAuthMap)
		pathAuthMap = new UrlPathAuthMap(*info.pathAuthMap);
	else
		pathAuthMap = 0;
}

Client::HostInfo::~HostInfo() {
	delete cookies;
	delete pathAuthMap;
}


/* MessageCounts */

std::ostream &MessageCounts::print(ostream &os) const {
	os << requests << " request headers sent";
	if (responses)
		os << ", " << responses << " final response headers received";
	if (authed)
		os << ", and " << authed << " responses authed";
	return os;
}

bool operator ==(const MessageCounts &m1, const MessageCounts &m2)
{
	return m1.requests == m2.requests &&
		m1.responses == m2.responses &&
		m1.authed == m2.authed;
}

