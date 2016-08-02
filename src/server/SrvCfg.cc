
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
#include "runtime/LogComment.h"
#include "runtime/polyErrors.h"
#include "csm/ContentCfg.h"
#include "csm/ContentMgr.h"
#include "pgl/ServerSym.h"
#include "server/Server.h"
#include "server/SrvCfg.h"
#include "csm/RamFiles.h"

SrvSharedCfgs TheSrvSharedCfgs;

SrvCfg::SrvCfg(): theServer(0), thePopModel(0), theRepTypeSel(0),
	theProtocolSel(0), theCookieCounts(0), theCookieSizes(0),
	theCookieSendProb(0), theReqBodyAllowed(-1) {
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
	configureContentsMap(ccfgs);

	configureCookies();

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

void SrvCfg::configureContentsMap(Array<ContentCfg*> &ccfgs) {
	for (int i = 0; i < ccfgs.count(); ++i) {
		const RamFiles *ramFiles = ccfgs[i]->ramFiles();
		if (!ramFiles)
			continue;
		for (int index = 0; index < ramFiles->count(); ++index) {
			const String &name = ramFiles->fileAt(index).name;
			const pair<RamFilesByName::iterator, bool> res =
				theRamFilesByName.insert(std::make_pair(name,
					std::make_pair(ccfgs[i], index)));
			if (!res.second) {
				static bool didOnce = false;
				if (!didOnce) {
					didOnce = true;
					Comment(1) << "warning: Same-server document_roots for " <<
						"Content " << ccfgs[i]->kind() << " and Content " <<
						res.first->second.first->kind() << " contain files " <<
						"with identical names: " << name << endc;
				}
			}
		}
	}
}

bool SrvCfg::sslActive(const int protocol) const {
	return protocol != Agent::pFTP && !theSslWraps.empty();
}

int SrvCfg::selectProtocol() {
	return theProtocolSel->ltrial();
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

bool SrvCfg::findRamFile(const String &fileName, ObjId &oid) const {
	const RamFilesByName::const_iterator it = theRamFilesByName.find(fileName);
	if (it == theRamFilesByName.end())
		return false;
	oid.type(it->second.first->id()); // ContentCfg ID
	oid.name(it->second.second + 1); // RamFile position in that ContentCfg + 1
	return true;
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

