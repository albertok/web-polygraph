
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
		virtual HttpAuthScheme proxyAuth() const;
		virtual int cookiesSent() const;
		virtual int cookiesRecv() const;

	protected:
//		virtual void noteHdrDataReady(bool &needMore);
//		virtual void noteBodyDataReady(bool &needMore);
//		virtual bool noteBufReady(WrBuf &buf);
		virtual void makeReq(WrBuf &buf);

		void makeConnectReq(ostream &os);
		void makeExplicitReq(ostream &os);
		void finishReqHdrs(ostream &os, bool forceDump);

		Error getHeader();
		Error interpretHeader();
		void getBody();
		void noteError(const Error &err);
		Error doForbidden();
		Error doProxyAuth();
		Error doOriginAuth();
		Error doAuth(const bool proxyAuth, const bool needed, Connection::NtlmAuth &ntlmState);
		Error startAuth(const HttpAuthScheme scheme);
		Error handleAuth();
		void redirect();

		virtual void finish(Error err);

		void makeReqVersion(ostream &os);
		void makeReqMethod(ostream &os);
		void makeEndToEndHdrs(ostream &os);
		void makeHopByHopHdrs(ostream &os);
		void makeCookies(ostream &os);
		void makeProxyAuthHdr(ostream &os);
		void makeOriginAuthHdr(ostream &os);
		void makeAuthHdr(const String &header, const HttpAuthScheme scheme, Connection::NtlmAuth &ntlmState, ostream &os);
		void makeAuthorization(const String &header, const HttpAuthScheme scheme, ostream &os);

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

		// this is used to check whether the xaction wants to use peers
		enum { peerUnknown = -1, peerNone, peerSome } thePeerState;

		enum { csNone, csWaiting, csDone } the100ContinueState;
};

#endif
