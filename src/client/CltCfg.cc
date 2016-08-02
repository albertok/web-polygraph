
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>
#include "xstd/h/iomanip.h"

#include "xstd/rndDistrs.h"
#include "xstd/StringIdentifier.h"
#include "xstd/Ssl.h"
#include "base/RndPermut.h"
#include "base/OidGenStat.h"
#include "pgl/RobotSym.h"
#include "pgl/SessionSym.h"
#include "pgl/AclSym.h"
#include "pgl/GoalSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglStaticSemx.h"
#include "pgl/SingleRangeSym.h"
#include "pgl/MultiRangeSym.h"
#include "runtime/AddrMap.h"
#include "runtime/HostMap.h"
#include "runtime/ObjUniverse.h"
#include "runtime/PopModel.h"
#include "runtime/Goal.h"
#include "runtime/LogComment.h"
#include "runtime/httpHdrs.h"
#include "csm/XmlTagIdentifier.h"
#include "csm/ContentMgr.h"
#include "csm/ContentSel.h"
#include "client/WarmupPlan.h"
#include "client/ServerRep.h"
#include "client/RegExGroup.h"
#include "client/ForeignWorld.h"
#include "client/Client.h"
#include "client/CltCfg.h"
#include "client/CltOpts.h"
#include "client/SingleRangeCfg.h"
#include "client/MultiRangeCfg.h"

Memberships CltCfg::TheGlbMemberships;


CltCfg::CltCfg(): theRobot(0), 
	theOriginSel(0), theReqTypeSel(0), theReqMethodSel(0),
	thePopModel(0),
	theBusyPeriod(0), theIdlePeriodDur(0),
	theFtpProxyCycleCnt(0), theHttpProxyCycleCnt(0), theSocksProxyCycleCnt(0),
	thePipelineDepth(0),
	theRecurRatio(-1),
	theSpnegoAuthRatio(-1),
	theEmbedRecurRatio(-1),
	theAuthError(0),
	theWaitXactLmt(-1), theIcpPort(-1),
	theCredentialCycleCnt(0), theForeignWorld(0),
	theContainerTags(0),
	theAcceptedContentCodings(0), acceptingGzipContent(false),
	theCookiesKeepLimitSel(0),
	theWarmupPlan(0),
	theReqBodyPauseProb(-1),
	theReqBodyRecurrence(-1),
	thePostContentSel(0), theUploadContentSel(0),
	theForeignInterestProb(-1),
	thePublicInterestProb(-1),
	thePassiveFtp(-1), theSocksProb(-1), theSocksChainingProb(-1),
	genUniqUrls(false), didWarmup(false), theRangeSel(0),
	ftpProxiesSet(false) {
}

CltCfg::~CltCfg() {
	while (theCredentials.count())
		delete theCredentials.dequeue();
	while (theFtpProxies.count())
		delete theFtpProxies.dequeue();
	while (theHttpProxies.count())
		delete theHttpProxies.dequeue();
	while (theSocksProxies.count())
		delete theSocksProxies.dequeue();
	delete theOriginSel;
	delete theReqTypeSel;
	delete theReqMethodSel;
	delete thePopModel;
	delete theBusyPeriod;
	delete theContainerTags;
	delete theAcceptedContentCodings;
	delete theForeignWorld;
	delete theWarmupPlan;
	delete thePostContentSel;
	delete theUploadContentSel;
	delete theRangeSel;
}

void CltCfg::configure(const RobotSym *aRobot) {
	AgentCfg::configure(aRobot);

	Assert(!theRobot && aRobot);
	theRobot = aRobot;

	theRobot->waitXactLimit(theWaitXactLmt);
	thePeerHttp = theRobot->peerHttp();
	thePeerIcp = theRobot->peerIcp();
	theRobot->icpPort(theIcpPort);

	theRobot->recurRatio(theRecurRatio);
	theRobot->spnegoRatio(theSpnegoAuthRatio);
	theRobot->embedRecurRatio(theEmbedRecurRatio);
	theRobot->authError(theAuthError);
	theRobot->uniqueUrls(genUniqUrls);
	theUriThrower = theRobot->rawUriThrower();
	theRobot->reqBodyPauseProb(theReqBodyPauseProb);
	theRobot->reqBodyPauseStart(theReqBodyPauseStart);
	theRobot->reqBodyRecurrence(theReqBodyRecurrence);

	configurePopModel();
	configureInterests();
	configureReqTypes();
	configureReqMethods();
	configureOrigins();
	configureProxies();
	configureCredentials();
	configureMemberships();
	configureContainerTags();
	configureAcceptedContentCodings();
	configurePipeDepth();
	configureRanges();
	configurePostContents();
	configureUploadContents();
	configureTrace();

	if (AclSym *acl = theRobot->acl())
		theAcl.configure(*acl);

	if (SessionSym *ss = theRobot->session()) {
		if (GoalSym *bps = ss->busyPeriod()) {
			theBusyPeriod = new Goal;
			theBusyPeriod->configure(*bps);
		}
		theIdlePeriodDur = ss->idlePeriodDuration();
		ss->heartbeatGap(theSessionHeartbitGap);

		if (!theBusyPeriod != !theIdlePeriodDur) {
			cerr << theRobot->loc() << "both busy and idle periods "
				<< "should be specified (or not specified) for a robot session"
				<< endl << xexit;
		}
	}

	if (!thePopModel && theRecurRatio > 0)
		cerr << theRobot->loc() << "popularity model must be specified for"
			<< " positive recurrence ratio (robot " << theRobot->kind() << ')'
			<< endl << xexit;

	if (!theRobot->cookieSender(theCookieSenderProb))
		theCookieSenderProb = 1;
	theCookiesKeepLimitSel = theRobot->cookiesKeepLimit();

	if (theReqBodyPauseProb >= 0 &&
		theReqBodyPauseStart >= 0)
		cerr << theRobot->loc() << "Both req_body_pause_prob and req_body_pause_start"
			<< " should not be specified (robot " << theRobot->kind() << ')'
			<< endl << xexit;

	theRobot->passiveFtp(thePassiveFtp);

	checkConfiguration();
}

int CltCfg::viservLimit() const {
	return TheHostMap->iterationCount();
}

void CltCfg::configurePopModel() {
	if (PopModelSym *const pms = theRobot->popModel()) {
		thePopModel = new PopModel;
		thePopModel->configure(pms);
	}
}

void CltCfg::configureInterests() {
	Array<StringSym*> istrs;
	Array<double> iprobs;
	if (theRobot->interests(istrs, iprobs)) {
		double pubProb = 0;
		double privProb = 0;
		for (int i = 0; i < istrs.count(); ++i)
			if (istrs[i]->val() == "foreign")
				theForeignInterestProb = iprobs[i];
                        else
			if (istrs[i]->val() == "public")
				pubProb = iprobs[i];
                        else
			if (istrs[i]->val() == "private")
				privProb = iprobs[i];
			else {
				cerr << theRobot->loc() << "interests array can "
					"have only \"foreign\", \"private\" and "
					"\"public\" values" << endl << xexit;
			}
		if (privProb > 0) {
			thePublicInterestProb = pubProb / (privProb + pubProb);
			Comment(1) << theRobot->loc() << "private Robot URLs are "
				"currently not supported, private interest is "
				"treated as additional public interest" << endc;
		} else
		if (pubProb > 0)
			thePublicInterestProb = 1;
		// XXX: private worlds are not supported for now
		thePublicInterestProb = 1;
	}
}

void CltCfg::configureReqTypes() {
	static StringIdentifier sidf;
	if (!sidf.count()) {
		sidf.add("Basic", Client::rqtBasic);
		sidf.add("Ims200", Client::rqtIms200);
		sidf.add("Ims304", Client::rqtIms304);
		sidf.add("Reload", Client::rqtReload);
		sidf.add("Range", Client::rqtRange);
		sidf.add("Upload", Client::rqtUpload);
	}

	theReqTypeSel = theRobot->msgTypes(sidf);
	if (!theReqTypeSel)
		theReqTypeSel = new ConstDistr(0, Client::rqtBasic); // default
	theReqTypeSel->rndGen(LclRndGen("client_req_types"));
}

void CltCfg::configurePipeDepth() {
	thePipelineDepth = theRobot->pipelineDepth();
	if (thePipelineDepth)
		thePipelineDepth->rndGen(LclRndGen("client_pipe_depth"));
}

void CltCfg::configureReqMethods() {
	static StringIdentifier sidf;
	if (!sidf.count()) {
		sidf.add("GET", Client::rqmGet);
		sidf.add("HEAD", Client::rqmHead);
		sidf.add("POST", Client::rqmPost);
		sidf.add("PUT", Client::rqmPut);
	}

	theReqMethodSel = theRobot->reqMethods(sidf);
	if (!theReqMethodSel)
		theReqMethodSel = new ConstDistr(0, Client::rqmGet); // default
	theReqMethodSel->rndGen(LclRndGen("client_req_methods"));
}

void CltCfg::configureOrigins() {
	PtrArray<NetAddr*> origNames;

	RndDistr *iad = 0;
	if (!theRobot->origins(origNames, theOriginSel) || !origNames.count()) {
		const bool isActive = !theRobot->reqInterArrival(iad) || iad;
		if (isActive && theForeignInterestProb < 1)
			Comment(0) << theRobot->loc() << "no origin addresses "
				"specified for active Robot '" <<
				theRobot->kind() << "' with some non-foreign "
				"interest" << endc << xexit;
		return;
	}
	if (theForeignInterestProb >= 1) {
		Comment(0) << theRobot->loc() << "origin addresses specified "
			"for Robot '" << theRobot->kind() << "' with "
			"foreign-only interest" << endc;
		delete theOriginSel;
		theOriginSel = 0;
		return;
	}

	Assert(theOriginSel);
	theOriginSel->rndGen(LclRndGen("client_origins"));

	theViservs.stretch(origNames.count());
	for (int i = 0; i < origNames.count(); ++i)
		addOrigName(*origNames[i]);
}

// converts origin name into viserv idx
void CltCfg::addOrigName(const NetAddr &oname) {
	if (!TheAddrMap->has(oname)) {
		if (oname.isDomainName())
			cerr << here << "visible server name " << oname
				<< " is not found in address maps" << endl << xexit;
		TheAddrMap->add(oname);
	}

	int viserv = -1;
	if (!TheHostMap->find(oname, viserv))
		TheHostMap->addAt(viserv, oname);
	HostCfg *host = TheHostMap->at(viserv);
	if (!host->theUniverse)
		ObjUniverse::Add(host, new ObjUniverse(UniqId::Create()));

	if (!host->theServerRep)
		host->theServerRep = new ServerRep(oname, viserv);

	// quit if an origin entry is repeated because it complicates
	// private world accounting (two origins would have one world)
	if (hasViserv(viserv)) {
		Comment << theRobot->loc() << "error: origin " << oname <<
			" is listed more than once in robot's origins" << 
			endc << xexit;
	}

	theViservs.append(viserv);

	checkTargets(viserv);
}

// checks content cfg and creates new server representative for viserv
void CltCfg::checkTargets(int viserv) {
	const NetAddr &visName = TheHostMap->at(viserv)->theAddr;
	int niamIdx; // name in AddrMap index
	Assert(TheAddrMap->find(visName, niamIdx));

	for (AddrMapAddrIter i = TheAddrMap->addrIter(niamIdx); i; ++i) {
		const NetAddr &addr = i.addr();
		int targetIdx = -1;
		if (!TheHostMap->find(addr, targetIdx) ||
			!TheHostMap->at(targetIdx)->theContent) {
			Comment << theRobot->loc() << "error: Robot cannot find"
				<< " configuration for server@" << addr;
			if (addr != i.name())
				Comment << " (visible as " << i.name() << ")";
			Comment << endc << xexit;
		}
	}
}

void CltCfg::checkConfiguration() const {
	// collect reachable contents
	ContentSet contents;
	for (int i = 0; i < theViservs.count(); ++i) {
		const NetAddr &visName = TheHostMap->at(theViservs[i])->theAddr;
		int niamIdx; // name in AddrMap index
		Assert(TheAddrMap->find(visName, niamIdx));
		for (AddrMapAddrIter j = TheAddrMap->addrIter(niamIdx); j; ++j) {
			int targetIdx = -1;
			Assert(TheHostMap->find(j.addr(), targetIdx));
			const HostCfg *const hcfg = TheHostMap->at(targetIdx);
			Assert(hcfg && hcfg->theContent);
			const Array<ContentCfg*> &srvContents =
				hcfg->theContent->contents();
			for (int k = 0; k < srvContents.count(); ++k)
				contents.insert(srvContents[k]);
		}
	}

	checkPostContentsConfiguration(contents);
	checkUploadContentsConfiguration(contents);
	checkRangesConfiguration(contents);
}

void CltCfg::checkPostContentsConfiguration(const ContentSet &contents) const {
	const bool havePostContents = thePostContentSel;
	if (havePostRequest()) {
		if (!havePostContents) {
			Comment(0) << theRobot->loc() << "'POST' request method"
				" configured but no 'post_contents' set"
				<< endc << xexit;
		}
		return;
	}
	bool noPostRequest = true;
	for (ContentSet::const_iterator i = contents.begin();
		i != contents.end(); ++i) {
		const ContentCfg *const content = *i;
		Assert(content);
		const CltCfg *const behavior =
			TheCltBehaviorCfgs.get(content->id());
		if (!behavior || !behavior->havePostRequest())
			continue;
		if (!havePostContents) {
			Comment(0) << behavior->theRobot->loc() << "'POST' "
				"request method configured for Content '" <<
				content->kind() << "' but Robot '" <<
				theRobot->kind() << "' has no 'post_contents' "
				"set" << endc << xexit;
		}
		noPostRequest = false;
	}
	if (havePostContents && noPostRequest) {
		Comment(0) << theRobot->loc() << "'post_contents' set but no "
			"'POST' request method configured" << endl << xexit;
	}
}

void CltCfg::checkUploadContentsConfiguration(const ContentSet &contents) const {
	const bool haveUploadContents = theUploadContentSel;
	if (haveUploadRequest()) {
		if (!haveUploadContents) {
			Comment(0) << theRobot->loc() << "'Upload' request "
				"type or 'PUT' request method configured but no"
				" 'upload_contents' set" << endl << xexit;
		}
		return;
	}
	bool noUploadRequest = true;
	for (ContentSet::const_iterator i = contents.begin();
		i != contents.end(); ++i) {
		const ContentCfg *const content = *i;
		Assert(content);
		const CltCfg *const behavior =
			TheCltBehaviorCfgs.get(content->id());
		if (!behavior || !behavior->haveUploadRequest())
			continue;
		if (!haveUploadContents) {
			Comment(0) << behavior->theRobot->loc() << "'Upload' "
				"request type or 'PUT' request method "
				"configured for Content '" << content->kind()
				<< "' but Robot '" << theRobot->kind() << "' "
				"has no 'upload_contents' set" << endc << xexit;
		}
		noUploadRequest = false;
	}
	if (haveUploadContents && noUploadRequest) {
		Comment(0) << theRobot->loc() << "'upload_contents' set but no "
			"'Upload' request type or 'PUT' request method "
			"configured" << endl << xexit;
	}
}

void CltCfg::checkRangesConfiguration(const ContentSet &contents) const {
	const bool haveRanges = theRangeSel;
	if (haveRangeRequest() && haveRanges)
		return;
	bool noRangeRequest = true;
	bool allRanges = true;
	for (ContentSet::const_iterator i = contents.begin();
		i != contents.end(); ++i) {
		const ContentCfg *const content = *i;
		Assert(content);
		const CltCfg *const behavior =
			TheCltBehaviorCfgs.get(content->id());
		if (!behavior) {
			allRanges = false;
			continue;
		}
		const bool needRanges = behavior->theReqTypeSel ?
			behavior->haveRangeRequest() : haveRangeRequest();
		if (needRanges && !behavior->theRangeSel)
			allRanges = false;
		if (behavior->theRangeSel && !needRanges) {
			Comment(0) << behavior->theRobot->loc() <<
				"'ranges' set but no 'Range' request type "
				"configured for Content '" << content->kind() <<
				"' and Robot '" << theRobot->kind() << '\''
				<< endl << xexit;
		}
		if (!haveRanges && !behavior->theRangeSel && needRanges) {
			Comment(0) << behavior->theRobot->loc() << "'Range' "
				"request type configured but no 'ranges' set "
				"for Content '" << content->kind() << "' and "
				"Robot '" << theRobot->kind() << '\''
				<< endc << xexit;
		}
		if (behavior->haveRangeRequest())
			noRangeRequest = false;
	}
	if (haveRanges && noRangeRequest) {
		Comment(0) << theRobot->loc() << "'ranges' set but no 'Range' "
			"request type configured" << endl << xexit;
	}
	if (haveRangeRequest() && !allRanges) {
		Comment(0) << theRobot->loc() << "'Range' request "
			"type configured but no 'ranges' set"
			<< endl << xexit;
	}
}

bool CltCfg::havePostRequest() const {
	return theRobot->arrayHasElem("req_methods", "POST");
}

bool CltCfg::haveUploadRequest() const {
	return theRobot->arrayHasElem("req_types", "Upload") ||
		theRobot->arrayHasElem("req_methods", "PUT");
}

bool CltCfg::haveRangeRequest() const {
	return theRobot->arrayHasElem("req_types", "Range");
}

void CltCfg::configureProxies() {
	Array<NetAddr*> addrs;

	ftpProxiesSet = theRobot->ftpProxies(addrs);
	if (ftpProxiesSet) {
		theFtpProxies.resize(addrs.count());
		while (addrs.count())
			theFtpProxies.enqueue(addrs.pop());
	}

	if (theRobot->httpProxies(addrs)) {
		theHttpProxies.resize(addrs.count());
		while (addrs.count())
			theHttpProxies.enqueue(addrs.pop());
		if (TheCltOpts.theProxyAddr)
			Comment << theRobot->loc() << "--proxy option and "
				<< "Robot.http_proxies field are mutually exclusive" << endc << xexit;
	}

	if (theRobot->proxies(addrs)) {
		Comment(1) << theRobot->loc() << "Robot::proxies field is "
			<< "deprecated in favor of the Robot::http_proxies "
			<< "field. See also Robot::ftp_proxies" << endc;
		if (!theHttpProxies.empty())
			Comment << theRobot->loc() << "Robot.http_proxies and "
				<< "Robot.proxies field are mutually exclusive" << endc << xexit;
		if (TheCltOpts.theProxyAddr)
			Comment << theRobot->loc() << "--proxy option and "
				<< "Robot.proxies field are mutually exclusive" << endc << xexit;
		theHttpProxies.resize(addrs.count());
		while (addrs.count())
			theHttpProxies.enqueue(addrs.pop());
	}

	configureSocksProxies();
}

void CltCfg::configureSocksProxies() {
	Array<NetAddr*> addrs;
	const bool socksProbDefault = !theRobot->socksProb(theSocksProb);
	if (theRobot->socksProxies(addrs)) {
		theSocksProxies.resize(addrs.count());
		while (addrs.count())
			theSocksProxies.enqueue(addrs.pop());
		if (socksProbDefault)
			theSocksProb = 1;
	} else if (theSocksProb > 0) {
		cerr << theRobot->loc() << "Robot::socks_prob field "
			<< "is positive but no Robot::socks_proxies configured"
			<< endl << xexit;
	}
	const bool socksProxyPresent = !theSocksProxies.empty();
	const bool otherProxyPresent =
		!theHttpProxies.empty() || !theFtpProxies.empty();
	theRobot->socksChainingProb(theSocksChainingProb);
	if (theSocksChainingProb > 0) {
		if (!socksProxyPresent) {
			cerr << theRobot->loc()
				<< "Robot::socks_chaining_prob field is positive "
				<< "but no Robot::socks_proxies configured"
				<< endl << xexit;
		}
		if (!otherProxyPresent) {
			cerr << theRobot->loc()
				<< "Robot::socks_chaining_prob field is positive "
				<< "but no Robot::http_proxies/Robot::ftp_proxies "
				<< "configured"	<< endl << xexit;
		}
	}
}

void CltCfg::configureCredentials() {
	Array<String*> creds;
	theRobot->credentials(creds);
	theCredentials.resize(creds.count());
	while (creds.count())
		theCredentials.enqueue(creds.pop());
}

void CltCfg::configureContainerTags() {
	Array<String*> tags;
	if (!theRobot->containerTags(tags))
		tags.append(new String("<embed src>")); // default
	theContainerTags = new XmlTagIdentifier();
	theContainerTags->configure(tags);
	while (tags.count()) delete tags.pop();
}

void CltCfg::configureAcceptedContentCodings() {
	Array<String*> codings;
	if (theRobot->acceptedContentCodings(codings)) {
		theAcceptedContentCodings = new String;
		for (int i = 0; i < codings.count(); ++i) {
			const String &coding = *codings[i];
			if (coding.startsWith("*") || coding.startsWith("gzip"))
				acceptingGzipContent = true;
			if (*theAcceptedContentCodings)
				*theAcceptedContentCodings += ", ";
			*theAcceptedContentCodings += coding;
			delete codings[i];
		}
	}
}

void CltCfg::configureMemberships() {
	// initialize global array once
	const Array<MembershipMapSym*> &syms = PglStaticSemx::TheMembershipsToUse;
	if (TheGlbMemberships.count() != syms.count()) {
		TheGlbMemberships.stretch(syms.count());
		for (int i = 0; i < syms.count(); ++i) {
			MembershipMap *m = new MembershipMap;
			m->configure(*syms[i], i+1);
			TheGlbMemberships.append(m);
		}
	}

	for (int i = 0; i < TheGlbMemberships.count(); ++i) {
		MembershipMap *g = TheGlbMemberships[i];
		bool belongs = false;
		for (int u = 0; !belongs && u < theCredentials.count(); ++u) {
			belongs = g->hasMember(*theCredentials.top(u));
		}
		if (belongs)
			theLclMemberships.append(g);
	}
}

void CltCfg::configureTrace() {
	if (const String fname = theRobot->foreignTrace()) {
		if (theForeignInterestProb > 0) {
			theForeignWorld = new ForeignWorld;
			theForeignWorld->configure(fname);
		} else {
			Comment(1) << theRobot->loc() << "warning: foreign "
				<< "trace '" << fname << "' is configured "
				<< "but no foreign interest specified"
				<< endc;
		}
	} else
	if (theForeignInterestProb > 0) {
		Comment << theRobot->loc() << "error: foreign interest is "
			<< "positive but no foreign trace configured"
			<< endc << xexit;
	}
}

void CltCfg::configureRanges() {
	Array<const RangeSym*> syms;
	if (!theRobot->ranges(syms, theRangeSel))
		return; // no ranges configured

	theRangeSel->rndGen(LclRndGen("client_ranges"));
	theRanges.stretch(syms.count());
	for (int i = 0; i < syms.count(); ++i) {
		const RangeSym &sym = *syms[i];
		if (sym.isA(SingleRangeSym::TheType)) {
			SingleRangeCfg *const r = new SingleRangeCfg;
			r->configure((const SingleRangeSym&)sym);
			theRanges.append(r);
		}
		else
		if (sym.isA(MultiRangeSym::TheType)) {
			MultiRangeCfg *const r = new MultiRangeCfg;
			r->configure((const MultiRangeSym&)sym);
			theRanges.append(r);
		}
		else
			cerr
				<< theRobot->loc()
				<< "Unknown range type \"" << sym.type() << '"'
				<< endl << xexit;
	}
}

void CltCfg::configurePostContents() {
	Array<ContentSym*> syms;
	if (!theRobot->reqContents("post_contents", syms, thePostContentSel))
		return; // no contents configured

	thePostContentSel->rndGen(LclRndGen("client_post_contents"));
	thePostContents.stretch(syms.count());
	TheContentMgr.get(syms, thePostContents);
}

void CltCfg::configureUploadContents() {
	Array<ContentSym*> syms;
	if (!theRobot->reqContents("upload_contents", syms, theUploadContentSel)) {
		if (!theRobot->reqContents("put_contents", syms, theUploadContentSel))
			return; // no contents configured
		else {
			Comment(1) << theRobot->loc() << "Robot::put_contents field "
				"is deprecated in favor of the "
				"Robot::upload_contents field" << endc;
		}
	}

	theUploadContentSel->rndGen(LclRndGen("client_upload_contents"));
	theUploadContents.stretch(syms.count());
	TheContentMgr.get(syms, theUploadContents);
}

bool CltCfg::hasViserv(int viserv) const {
	for (int origin = 0; origin < theViservs.count(); ++origin) {
		if (theViservs[origin] == viserv)
			return true;
	}
	return false;
}

bool CltCfg::selectCredentials(String &newCred) {
	if (theCredentials.empty())
		return false;

	// randomize in the beginning of each cycle
	if (theCredentialCycleCnt >= theCredentials.count())
		theCredentialCycleCnt = 0;
	if (!theCredentialCycleCnt) {
		static RndGen rng;
		theCredentials.randomize(rng);
	}
	theCredentialCycleCnt++;
		
	String *credential = theCredentials.dequeue();
	theCredentials.enqueue(credential);
	newCred = *credential;
	return true;
}

int CltCfg::selectViserv() {
	if (theWarmupPlan) {
		const int viservIdx = theWarmupPlan->selectViserv();
		if (viservIdx >= 0)
			return viservIdx;
		stopWarmup();
	}

	const int viservIdx = (int)theOriginSel->trial();
	Assert(0 <= viservIdx && viservIdx < theViservs.count());
	return theViservs[viservIdx];
}

bool CltCfg::selectFtpProxy(NetAddr &newAddr) {
	if (!ftpProxiesSet)
		return false;

	selectProxy(theFtpProxies, theFtpProxyCycleCnt, newAddr);
	return true;
}

bool CltCfg::selectHttpProxy(NetAddr &newAddr) {
	if (TheCltOpts.theProxyAddr)
		newAddr = TheCltOpts.theProxyAddr;
	else
		selectProxy(theHttpProxies, theHttpProxyCycleCnt, newAddr);
	return true;
}

bool CltCfg::selectSocksProxy(NetAddr &newAddr, bool &doSocksChaining) {
	if (theSocksProb <= 0)
		return false;

	static RndGen *const rng1 = LclRndGen("socks_prob");
	if (!rng1->event(theSocksProb))
		return false;

	selectProxy(theSocksProxies, theSocksProxyCycleCnt, newAddr);
	if (theSocksChainingProb > 0) {
		static RndGen *const rng2 = LclRndGen("socks_chaining_prob");
		doSocksChaining = rng2->event(theSocksChainingProb);
	} else
		doSocksChaining = false;
	return true;
}

bool CltCfg::selectFtpMode(bool &usePassive) {
	if (thePassiveFtp < 0)
		return false;

	static RndGen rng;
	usePassive = rng.event(thePassiveFtp);
	return true;
}

int CltCfg::findMemberships(const String &user, Memberships &groups) const {
	if (user) {
		for (int i = 0; i < theLclMemberships.count(); ++i) {
			MembershipMap *g = theLclMemberships[i];
			if (g->hasMember(user))
				groups.append(g);
		}
	}
	return groups.count();
}

bool CltCfg::followAllUris(const RepHdr &rep) const {
	if (theUriThrower.len() > 0) {
		return theUriThrower == "*" ||
			rep.theServer.startsWith(theUriThrower);
	} else {
		return false;
	}
}

RangeCfg::RangesInfo CltCfg::makeRangeSet(HttpPrinter &hp, const ObjId &oid, ContentCfg &contentCfg) const {
	Assert(theRangeSel);
	return theRanges[theRangeSel->ltrial()]->makeRangeSet(hp, oid, contentCfg);
}

// may be called multiple times
void CltCfg::startWarmup() {
	if (!theWarmupPlan && !didWarmup && theForeignInterestProb < 1) {
		theWarmupPlan = new WarmupPlan(theViservs);

		Comment(7) << "started a warmup plan for Robot '" <<
			theRobot->kind() << "' with " <<
			theWarmupPlan->planCount() << " cold servers, out "
			"of " << ObjUniverse::Count() << " already visible" <<
			endc;
	}
}

void CltCfg::stopWarmup() {
	Assert(theWarmupPlan && !didWarmup);

	Comment(7) << "stopped warmup plan for Robot '" << theRobot->kind() <<
		"' with " << theWarmupPlan->planCount() << " cold servers, "
		"out of " << ObjUniverse::Count() << " globally visible" <<
		endc;

	const int visibleCnt = ObjUniverse::Count();
	static bool didOnce = false;
	if (!didOnce && WarmupPlan::ReadyCount() >= visibleCnt) {
		Comment(3) << "fyi: server scan completed with all local robots"
			<< " ready to hit all " << visibleCnt << " visible "
			"servers" << endc;
		didOnce = true;
	}

	delete theWarmupPlan;
	theWarmupPlan = 0;
	didWarmup = true;
}

void CltCfg::selectProxy(Ring<NetAddr*> &proxies, int &proxyCycleCnt, NetAddr &newAddr) {
	if (proxies.empty())
		return;

	// randomize in the beginning of each cycle
	if (proxyCycleCnt >= proxies.count())
		proxyCycleCnt = 0;
	if (!proxyCycleCnt) {
		static RndGen rng;
		proxies.randomize(rng);
	}
	proxyCycleCnt++;

	NetAddr *addr = proxies.dequeue();
	proxies.enqueue(addr);
	newAddr = *addr;
}

/* CltSharedCfgs */

CltSharedCfgs::~CltSharedCfgs() {
	while (count()) delete pop();
}

CltCfg *CltSharedCfgs::getConfig(const RobotSym *rs) {
	for (int i = 0; i < count(); ++i) {
		if (item(i)->theRobot == rs)
			return item(i);
	}
	return addConfig(rs);
}

CltCfg *CltSharedCfgs::addConfig(const RobotSym *rs) {
	CltCfg *cfg = new CltCfg;
	cfg->configure(rs);
	append(cfg);
	return cfg;
}
