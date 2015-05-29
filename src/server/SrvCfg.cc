
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/rndDistrs.h"
#include "xstd/StringIdentifier.h"
#include "xstd/h/iomanip.h"
#include "base/RndPermut.h"
#include "base/polyLogCats.h"
#include "runtime/HostMap.h"
#include "runtime/PopModel.h"
#include "runtime/ErrorMgr.h"
#include "runtime/XactAbortCoord.h"
#include "runtime/LogComment.h"
#include "runtime/polyErrors.h"
#include "csm/ContentCfg.h"
#include "csm/ContentMgr.h"
#include "pgl/ServerSym.h"
#include "server/Server.h"
#include "server/SrvCfg.h"

SrvSharedCfgs TheSrvSharedCfgs;

SrvCfg::SrvCfg(): theServer(0), thePopModel(0), theRepTypeSel(0),
	theProtocolSel(0), theCookieCounts(0), theCookieSizes(0),
	theCookieSendProb(0), theAbortProb(0), theReqBodyAllowed(-1) {
}

SrvCfg::~SrvCfg() {
	delete thePopModel;
	delete theRepTypeSel;
	delete theProtocolSel;
}

void SrvCfg::configure(const ServerSym *aServer) {
	AgentCfg::configure(aServer);

	Assert(!theServer && aServer);
	theServer = aServer;

	if (PopModelSym *pms = theServer->popModel()) {
		thePopModel = new PopModel;
		thePopModel->configure(pms);
	}

	configureRepTypes();
	configureProtocols();

	// configure content distributions
	Array<ContentSym*> csyms;
	Array<double> cprobs;
	Assert(theServer->contents(csyms, cprobs) && csyms.count());
	Array<ContentCfg*> ccfgs;
	TheContentMgr.get(csyms, ccfgs);
	theTypes.configure(ccfgs);

	configureCookies();

	theServer->abortProb(theAbortProb);
	theServer->reqBodyAllowed(theReqBodyAllowed);

	if (!theSslWraps.empty() &&
		theServer->arrayHasElem("protocol", "FTP")) {
		Comment(1) << theServer->loc() << "warning: FTPS protocol is "
			"not yet supported, ssl_wraps is currently ignored for "
			"FTP Servers" << endc;
	}
}

void SrvCfg::configureRepTypes() {
	static StringIdentifier sidf;
	if (!sidf.count()) {
		sidf.add("Basic", Server::rptBasic);
		sidf.add("302 Found", Server::rpt302Found);
	}

	theRepTypeSel = theServer->msgTypes(sidf);
	if (theRepTypeSel) {
		if (!theServer->popModel()) {
			cerr << theServer->loc() << "warning: some rep_types may require "
				<< "server popularity model and none was configured" << endl;
		}
	} else {
		// set to default
		theRepTypeSel = new ConstDistr(0, Server::rptBasic);
	}
	theRepTypeSel->rndGen(LclRndGen("server_req_types"));
}

void SrvCfg::configureProtocols() {
	static StringIdentifier sidf;
	if (!sidf.count()) {
		sidf.add("FTP" , Agent::pFTP);
		sidf.add("HTTP", Agent::pHTTP);
	}

	theProtocolSel = theServer->protocols(sidf);
	if (!theProtocolSel)
		theProtocolSel = new ConstDistr(0, Agent::pHTTP); // default

	static RndGen rng(GlbPermut(TheHostMap->hostCount(), rndProtocolSel));
	theProtocolSel->rndGen(&rng);
}

void SrvCfg::configureCookies() {
	theCookieCounts = theServer->cookieSetCount();
	theCookieSizes = theServer->cookieValueSize();
	if (theServer->cookieSetProb(theCookieSendProb)) {
		if (theCookieSendProb > 0 && (!theCookieCounts || !theCookieSizes)) {
			Comment << "error: cookie count and size distributions not " <<
				"specified but cookie_send probability is positive" << endc <<
				xexit;
		}
	} else {
		theCookieSendProb = 0; // no cookies by default
		if (theCookieCounts || theCookieSizes) {
			Comment << "error: cookie count and size distributions " <<
				"specified but cookie_send probability is not" << endc <<
				xexit;
		}
	}
	if (!theServer->cookieSender(theCookieSenderProb) &&
		theCookieSendProb > 0)
		theCookieSenderProb = 1;
}

bool SrvCfg::sslActive(const int protocol) const {
	return protocol != Agent::pFTP && !theSslWraps.empty();
}

int SrvCfg::selectProtocol() {
	return theProtocolSel->ltrial();
}

// merge this with CltCfg::selectAbortCfg() if more methods can be shared
void SrvCfg::selectAbortCoord(XactAbortCoord &coord) {
	static RndGen rng1, rng2; // uncorrelated unless theAbortProb is 1
	if (rng1.event(theAbortProb)) {
		const int whether = rng2.state();
		(void)rng2.trial();
		coord.configure(rng2.state(), whether);
	} else {
		const int whether = rng1.state();
		(void)rng1.trial();
		coord.configure(whether, rng1.state());
	}
}

bool SrvCfg::setEmbedContType(ObjId &oid, const Area &category) const {
	int etype = -1;
	if (theTypes.findEmbedContType(category, etype)) {
		oid.type(etype);
		return true;
	}

	if (ReportError2(errUnknownEmbedCategory, lgcSrvSide))
		Comment << "requested category: " << category << endc;
	return false;
}


/* SrvSharedCfgs */

SrvSharedCfgs::~SrvSharedCfgs() {
	while (count()) delete pop();
}

SrvCfg *SrvSharedCfgs::getConfig(const ServerSym *rs) {
	for (int i = 0; i < count(); ++i) {
		if (item(i)->theServer == rs)
			return item(i);
	}
	return addConfig(rs);
}

SrvCfg *SrvSharedCfgs::addConfig(const ServerSym *rs) {
	SrvCfg *cfg = new SrvCfg;
	cfg->configure(rs);
	append(cfg);
	return cfg;
}

