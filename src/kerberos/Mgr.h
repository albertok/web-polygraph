
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__KERBEROS_MGR_H
#define POLYGRAPH__KERBEROS_MGR_H

#include "xstd/Array.h"
#include "xstd/Kerberos.h"
#include "runtime/UserCred.h"
#include <deque>

class Client;
class CltXact;
class KerberosWrapSym;
class NetAddr;

namespace Kerberos {

class InitCredsXact;
class TicketXact;
class Xact;

class Mgr {
	public:
		Mgr(Client &anOwner, const KerberosWrapSym &cfg);

		void becomeBusy();
		void becomeIdle();
		bool createGssContext(const NetAddr &serviceAddr, CltXact &reason);

		const Client &owner() const { return theOwner; }
		const ::String &name() const { return theName; }
		const ::String &realm() const { return theRealm; }
		const Context &context() const { return theContext; }
		const Time &timeout() const { return theTimeout; }
		bool haveInitCreds() const { return acquiredInitCreds; }

		// enable if a single krb5 ccache cannot handle concurrent xactions
		bool atXactLimit() const { return false && theXactLevel > 0; }

		bool reportError(const ::Error err, const ErrorCode krbErr);

		void noteXactDone(InitCredsXact &x, bool isSuccessful);
		void noteXactDone(TicketXact &x, const bool isSuccessful);

		const Principal &principal() const { ShouldUs(!isIdle); return thePrincipal; }
		bool parsePrincipal(const ::String &name, Principal &principal);

		// [HTTP] authentication failed with the transaction ticket we provided
		// XXX: The failure may be due to an old TGT that we already replaced.
		void noteAuthFailure() { ++theUseFailures; }

		NetAddr *switchToTcp();

		// to filter very late xactions that end even after the idle gap ends
		int sessionId;
		bool usingTcp; // we will be using TCP for the next transaction(s)

	protected:
		void setDefaultPort(Array<NetAddr *> &kdcServers);
		const Array<NetAddr*> &servers() const; // picks the right theServers* array

		bool acquireInitCreds(const UserCred &cred);
		void restartCreateGssContext();
		void abortGssContextCreation();
		bool startCreateGssContext(const NetAddr &serviceAddr, CltXact &reason);

		bool late(const Xact &x) const;
		bool maybeRetryXact(Xact &x);
		int logCat() const;

		Client &theOwner; // Client object that uses us
		Context theContext; // Kerberos context
		::String theName; // client name
		Principal thePrincipal; // client principal
		CCache theCCache; // Kerberos credentials cache
		const ::String theRealm; // Kerberos realm
		PtrArray<NetAddr*> theServers; // KDC server addresses for TCP and UDP
		PtrArray<NetAddr*> theServersTcp; // KDC TCP server addresses
		PtrArray<NetAddr*> theServersUdp; // KDC UDP server addresses
		Time theTimeout; // KDC request timeout
		int theXactLevel; // the number of concurrently running transactions

		InitCredsXact *theInitCredsXact;
		Time theTicketEndTime; // TGT ticket expiration time
		int theUseFailures; // number of failed authentications with this TGT
		// xaction ticket transactions waiting for the TGT transaction; FIFO
		typedef std::deque< pair<const NetAddr*, CltXact*> > GssContextQueue;
		GssContextQueue theGssContextQueue;
		UserCred theCred;

		bool acquiredInitCreds; // true if acquired initial credentials
		bool isIdle; // the owner is idle; TODO: Expose Client::isIdle instead?
};

}; // namespace Kerberos

#endif
