
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iomanip.h"
#include "xstd/h/os_std.h"
#include "xstd/h/sys/stat.h"
#include "xstd/h/sys/types.h"
#include "xstd/Clock.h"

#include "client/Client.h"
#include "kerberos/Mgr.h"
#include "kerberos/InitCredsXact.h"
#include "kerberos/TicketXact.h"
#include "pgl/KerberosWrapSym.h"
#include "runtime/ErrorMgr.h"
#include "runtime/Farm.h"
#include "runtime/LogComment.h"
#include "runtime/polyErrors.h"

namespace Kerberos {

static ObjFarm<InitCredsXact> TheInitCredsXacts(1024); // magic, no good way to estimate
static ObjFarm<TicketXact> TheTicketXacts(1024); // magic, no good way to estimate

Mgr::Mgr(Client &anOwner, const KerberosWrapSym &cfg):
	sessionId(0),
	theOwner(anOwner),
	theRealm(cfg.realm()),
	theXactLevel(0),
	theInitCredsXact(0),
	theUseFailures(0),
	acquiredInitCreds(false),
	isIdle(true) {

	if (!Supported) {
		Comment(0) << cfg.loc() << "Polygraph was built without "
			"Kerberos support, cannot run Kerberos workloads" <<
			endc << xexit;
	}

	static bool didOnce = false;
	if (!didOnce) {
		didOnce = true;
		if (const char *fname = getenv("KRB5_CONFIG")) {
			Comment(1) << "warning: $KRB5_CONFIG (" << fname << ") " <<
				"settings may affect Polygraph operations." << endc;
		}
	}

	if (!theRealm) {
		Comment(0) << cfg.loc() << "no Kerberos realm specified"
			<< endc << xexit;
	}

	cfg.servers(theServers);
	cfg.serversTcp(theServersTcp);
	cfg.serversUdp(theServersUdp);

	setDefaultPort(theServers);
	setDefaultPort(theServersTcp);
	setDefaultPort(theServersUdp);

	if (theServers.empty() && theServersTcp.empty() && theServersUdp.empty()) {
		Comment(0) << cfg.loc() << "no KDC server addresses configured"
			<< endc << xexit;
	}
	if (!theServers.empty() && (!theServersTcp.empty() || !theServersUdp.empty())) {
		Comment(0) << cfg.loc() << "When configuring KDC server addresses, " <<
			"servers are mutually exclusive with servers_tcp and servers_udp" <<
			endc << xexit;
	}

	theTimeout = cfg.timeout();
	if (theTimeout < 0) {
		Comment(1) << cfg.loc() << "warning: no timeout for KDC "
			"requests specified" << endc;
	}

	if (const ErrorCode e = theContext.init()) {
		Comment(0) << cfg.loc() << "failed to create Kerberos "
			"context: " << Error(theContext, e) << endc << xexit;
	}

	if (const ErrorCode e = theCCache.init(theContext)) {
		Comment(0) << cfg.loc() << "failed to create Kerberos "
			"credentials cache: " << Error(theContext, e) << endc <<
			xexit;
	}

	usingTcp = theServersUdp.empty() && theServers.empty();
}

const Array<NetAddr *> &Mgr::servers() const {
	return !theServers.empty() ? theServers :
		(usingTcp ? theServersTcp : theServersUdp);
}

// acquire TGT
bool Mgr::acquireInitCreds(const UserCred &cred) {
	theTicketEndTime = Time();

	++sessionId;

	ShouldUs(!haveInitCreds());

	Must(cred.image());
	Must(cred.valid());
	bool hasRealm;
	theName = ::String(cred.name(hasRealm));
	if (!hasRealm)
		theName += "@" + realm();
	if (!parsePrincipal(name(), thePrincipal))
		return false;

	InitCredsXact *x = TheInitCredsXacts.get();
	Must(x);
	const ::String password(cred.password());
	if (!x->configure(*this, ::String(cred.password()))) {
		TheInitCredsXacts.put(x);
		return false;
	}

	theInitCredsXact = x;

	++theXactLevel;
	x->exec(*servers()[0]);
	return true;
}

bool Mgr::createGssContext(const NetAddr &serviceAddr, CltXact &reason) {
	if (theInitCredsXact) {
		// wait for the TGT transaction to finish before getting more tickets
		theGssContextQueue.push_back(make_pair(&serviceAddr, &reason));
		return true;
	}

	// if things went wrong and/or our TGT has expired, get a new TGT
	if (!haveInitCreds() || theUseFailures ||
		(theTicketEndTime > 0 && theTicketEndTime < TheClock)) {
		acquiredInitCreds = false; // may already be false
		if (!acquireInitCreds(theCred))
			return false;

		// wait for the TGT transaction to finish before getting the ticket
		// TODO: If the TGT transaction fails, queued transactions should
		// probably also fail with errKerberosNoTgt or similar.
		theGssContextQueue.push_back(make_pair(&serviceAddr, &reason));
		return true;
	}

	// In theory, we could get here from restartCreateGssContext() call that
	// resulted in Client launching more ticket transactions. Here, we try to
	// keep the transaction order the same by popping earlier transactions
	// first. TODO: We should not allow such reentrant behavior by breaking
	// the chain with an async call of some kind.
	restartCreateGssContext();

	return startCreateGssContext(serviceAddr, reason);
}
 
bool Mgr::startCreateGssContext(const NetAddr &serviceAddr, CltXact &reason) {
	TicketXact *const x = TheTicketXacts.get();
	Must(x);
	if (!x->configure(*this, theCCache, serviceAddr, reason)) {
		TheTicketXacts.put(x);
		return false;
	}

	++theXactLevel;
	x->exec(*servers()[0]);
	return true;
}

void Mgr::becomeIdle() {
	if (isIdle)
		return;

	isIdle = true;
	// TODO: we probably need to do more to fully clear the library state, but
	// there may still be pending transactions that need it
	thePrincipal.free();
	theName = String();
	acquiredInitCreds = false;
}

bool Mgr::reportError(const ::Error err, const ErrorCode krbErr) {
	if (ReportError(err)) {
		if (krbErr)
			Comment << Error(context(), krbErr) << endc;
		if (theXactLevel > 1)
			Comment << "concurrency level: " << theXactLevel << endc;
		return true;
	}
	return false;
}

bool Mgr::late(const Xact &x) const {
	return isIdle || x.sessionId != sessionId;
}

void Mgr::noteXactDone(InitCredsXact &x, bool isSuccessful) {
	ShouldUs(--theXactLevel >= 0);

	const bool isLate = late(x);

	if (!isLate && maybeRetryXact(x)) {
		Must(!isSuccessful);
		return;
	}

	if (theInitCredsXact == &x)
		theInitCredsXact = 0;

	if (!isLate && isSuccessful) {
		Must(theName);
		Must(thePrincipal);

		ErrorCode err = 0;
		Creds creds;
		if (!Should(!(err = theCCache.initialize(principal()))) ||
			!Should(!(err = x.getCreds(creds))) ||
			!Should(!(err = theCCache.store(creds)))) {
			isSuccessful = false;
			reportError(errKerberosCredsInit, err);
		}

		// even if we failed above, we got a new TGT so reset related state
		theUseFailures = 0;
		theTicketEndTime = x.ticketEndTime();

		acquiredInitCreds = isSuccessful;
	}

	TheInitCredsXacts.put(&x);

	if (acquiredInitCreds)
		restartCreateGssContext();
	else
		abortGssContextCreation();
}

void Mgr::restartCreateGssContext() {
	while (!theGssContextQueue.empty()) {
		const NetAddr &serviceAddr = *theGssContextQueue.front().first;
		CltXact &reason = *theGssContextQueue.front().second;
		theGssContextQueue.pop_front();
		if (!startCreateGssContext(serviceAddr, reason))
			theOwner.noteGssContext(reason, false);
	}
}

void Mgr::abortGssContextCreation() {
	// noteGssContext() calls may queue new xactions; abort only the old ones
	GssContextQueue::size_type debt = theGssContextQueue.size();
	while (debt-- > 0 && !theGssContextQueue.empty()) {
		CltXact &reason = *theGssContextQueue.front().second;
		theGssContextQueue.pop_front();
		theOwner.noteGssContext(reason, false);
	}
}

void Mgr::noteXactDone(TicketXact &x, const bool isSuccessful) {
	ShouldUs(--theXactLevel >= 0);

	const bool isLate = late(x);

	if (!isLate && maybeRetryXact(x)) {
		Must(!isSuccessful);
		return;
	}

	CltXact &reason = x.reason();
	reason.kdcAddr(x.kdcAddr());

	// TODO: move to future Owner::noteGssContext(reason, success ? &cache : 0)
	if (!isLate && isSuccessful) {
		if (!reason.initGssContext(theCCache, x.serviceName()))
			++theUseFailures;
	}

	TheTicketXacts.put(&x);
	theOwner.noteGssContext(reason, !isLate && isSuccessful);
}

bool Mgr::parsePrincipal(const ::String &name, Principal &principal) {
	if (const ErrorCode e = principal.init(context(), name)) {
		if (reportError(errKerberosPrincipal, e))
			Comment << "principal name: " << theName << endc;
		return false;
	}
	return true;
}

void Mgr::becomeBusy() {
	isIdle = false;
	// Use Client::credentials() directly instead of genCredentials()
	// because we do not want invalid credentials for Kerberos.
	const UserCred cred(owner().credentials());
	if (!cred.hasMacros()) {
		if (acquireInitCreds(cred)) {
			theCred = cred;
			return;
		}
		ReportError(errKerberosAsInit);
	} else if (ReportError(errKerberosCredsWithMacros)) {
		owner().describe(Comment(3) << "client: ");
		Comment << endc;
		Comment(3) << "credentials: " << owner().credentials() << endc;
	}
	// Damn the torpedoes! Let us hope Kerberos authentication is unused.
}

NetAddr *Mgr::switchToTcp() {
	if (!theServersTcp.empty() || !theServers.empty()) {
		usingTcp = true; // may already be true if another xaction switched us
		return servers()[0];
	}
	return NULL;
}

bool Mgr::maybeRetryXact(Xact &x) {
	if (x.needRetry()) {
		if (x.tryCount() < servers().count()) {
			++theXactLevel;
			x.exec(*servers()[x.tryCount()]);
			return true;
		}
		if (servers().count() > 1)
			ReportError(errKerberosAllSrvsFailed);
	}
	return false;
}

int Mgr::logCat() const {
	return owner().logCat();
}

void Mgr::setDefaultPort(Array<NetAddr*> &kdcServers) {
	for (int i = 0; i < kdcServers.count(); ++i) {
		Must(kdcServers[i]);
		if (kdcServers[i]->port() < 0)
			kdcServers[i]->port(88);
	}
}

}; // namespace Kerberos
