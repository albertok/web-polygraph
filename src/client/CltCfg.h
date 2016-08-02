
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CLTCFG_H
#define POLYGRAPH__CLIENT_CLTCFG_H

#include <set>
#include "xstd/Ring.h"
#include "xstd/BigSize.h"
#include "xstd/NetAddr.h"
#include "runtime/AgentCfg.h"
#include "client/MembershipMap.h"
#include "client/AclGroup.h"
#include "client/RangeCfg.h"

class Goal;
class ObjId;
class RndDistr;
class PopModel;
class RepHdr;
class RobotSym;
class WarmupPlan;
class RegExGroup;
class ForeignWorld;
class XmlTagIdentifier;
class ContentCfg;
class HttpPrinter;

// Client configuration items that can be shared among multiple clients
class CltCfg: public AgentCfg {
	public:
		CltCfg();
		~CltCfg();

		void configure(const RobotSym *aRobot);

		int viservLimit() const;
		int viservCount() const { return theViservs.count(); }
		bool hasViserv(int viserv) const;

		void startWarmup();
		void stopWarmup();

		int selectViserv();
		bool selectFtpProxy(NetAddr &addr);
		bool selectHttpProxy(NetAddr &addr);
		bool selectSocksProxy(NetAddr &addr, bool &doSocksChaining);
		bool selectCredentials(String &newName);
		bool selectFtpMode(bool &usePassive);

		bool followAllUris(const RepHdr &rep) const;
		int findMemberships(const String &user, Memberships &memberships) const;
		const AclGroup &acl() const { return theAcl; }
		ForeignWorld *foreignWorld() { return theForeignWorld; }

		RangeCfg::RangesInfo makeRangeSet(HttpPrinter &hp, const ObjId &oid, ContentCfg &contentCfg) const;

	protected:
		void configurePopModel();
		void configureInterests();
		void configureReqTypes();
		void configureReqMethods();
		void configureOrigins();
		void configureProxies();
		void configureSocksProxies();
		void configureCredentials();
		void configureMemberships();
		void configureContainerTags();
		void configureAcceptedContentCodings();
		void configurePipeDepth();
		void configureTrace();
		void configureRanges();
		void configurePostContents();
		void configureUploadContents();

		void addOrigName(const NetAddr &oname);
		void checkTargets(int viserv);

		typedef std::set<const ContentCfg*> ContentSet;
		void checkConfiguration() const;
		void checkPostContentsConfiguration(const ContentSet &contents) const;
		void checkUploadContentsConfiguration(const ContentSet &contents) const;
		void checkRangesConfiguration(const ContentSet &contents) const;
		bool havePostRequest() const;
		bool haveUploadRequest() const;
		bool haveRangeRequest() const;

		static void selectProxy(Ring<NetAddr*> &proxies, int &proxyCycleCnt, NetAddr &newAddr);

	protected:
		static Memberships TheGlbMemberships;

	public:
		const RobotSym *theRobot;  // used to identify/share configs

		Array<int> theViservs;     // origin idx to HostMap idx (viserv)
		RndDistr *theOriginSel;    // selects origin idx
		RndDistr *theReqTypeSel;   // selects request types
		RndDistr *theReqMethodSel; // selects request methods
		PopModel *thePopModel;     // popularity model to select old oids

		Goal *theBusyPeriod;       // sessions's busy period config
		RndDistr *theIdlePeriodDur;// sessions's idle period config
		Time theSessionHeartbitGap;// hearbit interval

		Ring<NetAddr*> theFtpProxies;
		Ring<NetAddr*> theHttpProxies;
		Ring<NetAddr*> theSocksProxies;
		int theFtpProxyCycleCnt;      // to randomize proxy selection
		int theHttpProxyCycleCnt;      // to randomize proxy selection
		int theSocksProxyCycleCnt;    // to randomize proxy selection

		RndDistr *thePipelineDepth;// maximum concurrent pipelined requests

		NetAddr thePeerHttp;       // caching peer talking HTTP
		NetAddr thePeerIcp;        // caching peer talking ICP
		double thePublicRatio;     // how often to request a "public" object
		double theRecurRatio;      // how often to re-visit an old object
		double theSpnegoAuthRatio; // how often negotiate/spnego auth is preferred over NTLMSSP
		double theEmbedRecurRatio; // how often to re-visit an old embedded obj
		double theAuthError;       // prob of an authentication error

		int theWaitXactLmt;        // limit on the number of postponed requests
		int theIcpPort;            // IPC client should use this port

		Memberships theLclMemberships; // all user groups users belong to
		Ring<String*> theCredentials;  // all users under this cfg
		int theCredentialCycleCnt;
		AclGroup theAcl;

		String theUriThrower;          // follow all URIs from this server
		ForeignWorld *theForeignWorld; // foreign URLs trace, if any

		XmlTagIdentifier *theContainerTags;
		String *theAcceptedContentCodings; // for Accept-Encoding header
		bool acceptingGzipContent;         // pre-parsed status

		RndDistr *theCookiesKeepLimitSel; // how many cookies to keep

		WarmupPlan *theWarmupPlan;

		double theReqBodyPauseProb; // how often to send "Expect: 100-continue"
		BigSize theReqBodyPauseStart;  // body size limt to send "Expect: 100-continue"

		double theReqBodyRecurrence;
		Array<ContentCfg*> thePostContents;
		RndDistr *thePostContentSel;
		Array<ContentCfg*> theUploadContents;
		RndDistr *theUploadContentSel;

		double theForeignInterestProb;
		double thePublicInterestProb;

		double thePassiveFtp; // probability of a robot using passive FTP
		double theSocksProb; // probability of a robot using SOCKS proxy
		double theSocksChainingProb; // probability of a SOCKS-using robot also using HTTP/FTP proxies

		bool genUniqUrls;          // each generated URL must be unique
		bool didWarmup;            // warmup plan was completed
		Array<const RangeCfg*> theRanges; // range configurations
		RndDistr *theRangeSel; // selects single range

		bool ftpProxiesSet; // true if Robot.ftp_proxies field is set
};

class CltSharedCfgs: protected Array<CltCfg*> {
	public:
		~CltSharedCfgs();
		CltCfg *getConfig(const RobotSym *cfg);

	protected:
		CltCfg *addConfig(const RobotSym *cfg);
};

#endif
