
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_CLTXACT_H
#define POLYGRAPH__CLIENT_CLTXACT_H

#include "runtime/Xaction.h"
#include "runtime/UserCred.h"

class Client;
class CltXactMgr;
class PipelinedCxm;
class DistrPoint;
class BodyParser;
class PageInfo;
class ParseBuffer;
class ServerRep;

namespace Kerberos {
class CCache;
};

class CltXact: public Xaction {
	public:
		CltXact();

		virtual void reset();

		void enqueue();

		// setup and execution
		void page(PageInfo *aPage);
		PageInfo *page();
		void owner(Client *const anOwner);
		virtual PipelinedCxm *getPipeline() = 0;
		virtual void pipeline(PipelinedCxm *aMgr) = 0;
		virtual void freezeProxyAuth() = 0;
		virtual void exec(Connection *const aConn);

		// called by CltXactMgr kids
		virtual bool controlledFill(bool &needMore);
		virtual bool controlledMasterWrite(Size &size);
		virtual bool controlledPostWrite(Size &size, bool &needMore);
		virtual bool controlledMasterRead();
		virtual bool controlledPostRead(bool &needMore) = 0;
		virtual void controlledAbort();

		virtual void cacheDistrPoint(DistrPoint *) {}
		virtual void noteAbort();
		virtual bool needRetry() const { return doRetry; }
		const UserCred &credentials() const { return theCred; }
		bool genCredentials();
		virtual bool needGssContext() const;
		void noteGssContext() { didGssContextAttempt = true; }
		bool initGssContext(const Kerberos::CCache &ccache, const String &target);
		void kdcAddr(const NetAddr &aKdcAddr) { theGssContext.kdcAddr(aKdcAddr); }

		const NetAddr &nextHighHop() const { return theNextHighHop; }
		void nextHighHop(const NetAddr &addr) { theNextHighHop = addr; }
		const NetAddr &nextTcpHop() const { return theNextTcpHop ? theNextTcpHop : theNextHighHop; }
		void nextTcpHop(const NetAddr &addr) { theNextTcpHop = addr; }

		CltXact *cause() { return theCause; }
		const CltXact *cause() const { return theCause; }
		void cause(CltXact *aCause) { theCause = aCause; }
		int childCount() const { return theChildCount; }
		virtual void noteChildNew(CltXact *child);
		virtual void noteChildGone(CltXact *);

		virtual bool askedPeer() const;
		virtual bool usePeer() const;
		virtual void usePeer(bool doUse);

		// called from BodyParsers
		virtual void noteContent(const ParseBuffer &content);
		virtual Error noteEmbedded(ReqHdr &hdr);
		virtual void noteTrailerHeader(const ParseBuffer &hdr);
		virtual void noteEndOfTrailer();
		virtual Error noteReplyPart(const RepHdr &hdr);

		virtual bool writeFirst() const;
		virtual bool startedXactSequence() const;
		virtual const CompoundXactInfo *partOf() const;

		virtual void wakeUp(const Alarm &a);

		// Authentication code works with either proxy or origin server. Some 
		// Kerberos/GSS code does not know the peer it is authenticating with
		// but is only used for proxy authentication and assumes that for now.
		typedef enum { aupUnknown, aupOrigin, aupProxy, aupAssumedProxy } AuthPeer;
		virtual void reportAuthError(const AuthPeer peer, const char *context, const Gss::Error gssErr = Gss::Error(), const Connection::NtlmAuth *ntlmState = 0);

	public:
		QueuePlace<CltXact> pipelinedXacts;

		// waiting for a connection slot (open_conn_lmt)
		QueuePlace<CltXact> waitingXacts;

	protected:
		virtual Agent *owner();

//		virtual void noteConnReady(bool &needMore);
//		virtual void noteHdrDataReady(bool &needMore) = 0;
//		virtual void noteBodyDataReady(bool &needMore);
//		virtual bool noteBufReady(WrBuf &buf);
		virtual void makeReq(WrBuf &buf) = 0;

		void parse();
		void checkOverflow();

		virtual void finish(Error err);
		virtual void logStats(OLog &ol) const;

		void consume(Size size);
		bool expectMore() const;
		Size unconsumed() const;
		bool validRelOid(const ObjId &oid) const;

		Gss::Context &gssContext(Connection::NtlmAuth &ntlmState);
		void startReportingAuthError(const char *context, Gss::Error gssErr = Gss::Error(), const Connection::NtlmAuth *ntlmState = 0);

	protected:
		Client *theOwner;
		PageInfo *thePage;
		CompoundXactInfo *theCompound; // we are a part of this compound xact
		BodyParser *theBodyParser;
		NetAddr theNextHighHop;
		NetAddr theNextTcpHop;
		CltXactMgr *theMgr;
		ServerRep *theSrvRep;

		UserCred theCred;     // authentication credentials
		Gss::Context theGssContext; // GSS authentication context

		// The transaction that caused this transaction. Causes include
		// xactions that needRetry() (e.g., after HTTP CONNECT or HTTP 407),
		// set oid.rediredReq(), or follow embedded links.
		CltXact *theCause;
		int theChildCount; // alive transactions this transaction has caused

		bool doRetry;		// try the same xaction again
		bool didGssContextAttempt; // tried to create GSS context
		bool createdCompound; // we created theCompound object
};

#endif
