
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_HTTPCLTXACT_H
#define POLYGRAPH__CLIENT_HTTPCLTXACT_H

#include "base/ObjTimes.h"
#include "client/CltXact.h"

class MembershipMap;
class RegExGroup;
class RegExMatchee;

class HttpCltXact: public CltXact {
	public:
		HttpCltXact();

		virtual void reset();

		// setup and execution
		virtual PipelinedCxm *getPipeline();
		virtual void pipeline(PipelinedCxm *aMgr);
		virtual void freezeProxyAuth();
		virtual bool needGssContext() const;
		virtual void exec(Connection *const aConn);

		// called by CltXactMgr
		virtual bool controlledFill(bool &needMore);
		virtual bool controlledPostRead(bool &needMore);
		virtual bool controlledPostWrite(Size &size, bool &needMore);

		// called from BodyParsers
		virtual void noteContent(const ParseBuffer &content);
		virtual Error noteEmbedded(ReqHdr &hdr);
		virtual void noteTrailerHeader(const ParseBuffer &hdr);
		virtual void noteEndOfTrailer();
		virtual Error noteReplyPart(const RepHdr &hdr);

		virtual bool askedPeer() const;
		virtual bool usePeer() const;
		virtual void usePeer(bool doUse);

		void saveRepHeader();
		const IOBuf &savedRepHeader() const { return theSavedRepHeader; }

		virtual int actualRepType() const { return theActualRepType; }
		virtual int cookiesSent() const;
		virtual int cookiesRecv() const;
		virtual AuthPhaseStat::Scheme proxyStatAuth() const;

	protected:
//		virtual void noteHdrDataReady(bool &needMore);
//		virtual void noteBodyDataReady(bool &needMore);
//		virtual bool noteBufReady(WrBuf &buf);
		virtual void makeReq(WrBuf &buf);

		void makeConnectReq(HttpPrinter &hp);
		void makeExplicitReq(HttpPrinter &hp);
		void finishReqHdrs(ostream &os, bool forceDump);

		Error getHeader();
		Error interpretHeader();
		void getBody();
		void noteError(const Error &err);
		Error doForbidden();
		Error doProxyAuth();
		Error doOriginAuth();
		Error doAuth(const bool proxyAuth, const bool needed, Connection::NtlmAuth &ntlmState, Gss::Error &gssErr);
		Error startAuth(const bool proxyAuth, const HttpAuthScheme scheme);
		Error handleAuth();
		void checkAuthEnd(const bool proxyAuth, const bool needed, const Error &err);
		virtual void reportAuthError(const AuthPeer peer, const char *context, const Gss::Error gssErr = Gss::Error(), const Connection::NtlmAuth *ntlmState = 0);
		void redirect();

		virtual void finish(Error err);

		void makeReqVersion(ostream &os);
		void makeReqMethod(ostream &os);
		void makeEndToEndHdrs(HttpPrinter &hp);
		void makeHopByHopHdrs(HttpPrinter &hp);
		void makeCookies(ostream &os);
		void makeProxyAuthHdr(HttpPrinter &hp);
		void makeOriginAuthHdr(HttpPrinter &hp);
		void makeAuthHdr(const bool proxyAuth, const String &header, const HttpAuthScheme scheme, Connection::NtlmAuth &ntlmState, HttpPrinter &hp);
		bool makeAuthorization(const String &header, const HttpAuthScheme scheme, HttpPrinter &hp);

		void firstHandSync();

		Error handleEmbedOid(ReqHdr &hdr);
		Error handleForeignEmbedOid(ReqHdr &hdr);

		const ObjTimes &olcTimes() const;
		bool cfgAbortedReply() const;

		void checkAcl();
		void checkAclMatch(const RegExGroup *matchGrp, const char *action);
		void buildAclMatchee(RegExMatchee &m) const;
		void explainAclMatch(const RegExMatchee &m, const char *action, const Array<RegExGroup*> &ourMatches) const;
		void dumpMatchingGroupNames(ostream &os, const MembershipMap *map) const;

		void checkFreshness();
		void checkDateSync();
		Error setStatusCode(int aStatus);
		Error setViserv(const NetAddr &name, ObjId &oid) const;
		BodyParser *selectBodyParser();
		BodyParser *selectContentParser();
		BodyParser *selectMarkupBodyParser();

		RndDistr *seedOidDistr(RndDistr *raw, int globSeed);

		void createCompound();

	protected:
		RepHdr theRepHdr;
		mutable ObjTimes theOlcTimes; // cached value
		Size theReqBodySize;
		IOBuf theSavedRepHeader; // to preserve raw response headers if needed

		Size theRangesSize;
		Size theRangeCount;
		Size theBodyPartsSize;
		Size theBodyPartCount;

		int theActualRepType;

		HttpAuthScheme theProxyAuthScheme; // at the time of request making

		// this is used to check whether the xaction wants to use peers
		enum { peerUnknown = -1, peerNone, peerSome } thePeerState;

		enum { csNone, csWaiting, csDone } the100ContinueState;
};

#endif
