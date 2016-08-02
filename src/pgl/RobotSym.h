
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_ROBOTSYM_H
#define POLYGRAPH__PGL_ROBOTSYM_H

#include "xstd/Array.h"
#include "pgl/AgentSym.h"

class String;
class RndDistr;
class SessionSym;
class AclSym;
class DnsResolverSym;
class KerberosWrapSym;
class RangeSym;
class ContentSym;

// client side configuration
class RobotSym: public AgentSym {
	public:
		static const String TheType;

	public:
		RobotSym(const String &aType = TheType);
		RobotSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool reqInterArrival(RndDistr *&iad) const;
		bool recurRatio(double &ratio) const;
		bool spnegoRatio(double &ratio) const;
		bool embedRecurRatio(double &ratio) const;
		bool uniqueUrls(bool &set) const;
		bool openConnLimit(int &lmt) const;
		bool waitXactLimit(int &lmt) const;
		bool minimizeNewConn(double &prob) const;
		bool authError(double &prob) const;
		SessionSym *session() const;
		bool icpPort(int &port) const;
		NetAddr peerHttp() const;
		NetAddr peerIcp() const;
		AclSym *acl() const;
		DnsResolverSym *dnsResolver() const;
		bool privCache(int &capacity) const;
		bool proxies(Array<NetAddr*> &addrs) const;
		bool ftpProxies(Array<NetAddr*> &addrs) const;
		bool httpProxies(Array<NetAddr*> &addrs) const;
		bool socksProxies(Array<NetAddr*> &addrs) const;
		bool origins(Array<NetAddr*> &addrs) const;
		bool origins(Array<NetAddr*> &addrs, RndDistr *&sel) const;
		bool credentials(Array<String*> &creds) const;
		bool containerTags(Array<String*> &tags) const;
		bool acceptedContentCodings(Array<String*> &codings) const;
		bool interests(Array<StringSym*> &istrs, Array<double> &iprobs) const;
		RndDistr *interests(const TokenIdentifier &interestKinds) const;
		RndDistr *reqMethods(const TokenIdentifier &reqMethodNames) const;
		RndDistr *pipelineDepth() const;
		String foreignTrace() const;
		String rawUriThrower() const;
		RndDistr *cookiesKeepLimit() const;
		bool ranges(Array<const RangeSym*> &syms, RndDistr *&sel) const;
		bool reqBodyPauseProb(double &f) const;
		bool reqBodyPauseStart(BigSize &sz) const;

		bool reqBodyRecurrence(double &f) const;
		bool reqContents(const String &param, Array<ContentSym*> &syms, RndDistr *&sel) const;

		bool passiveFtp(double &prob) const;
		bool socksProb(double &prob) const;
		bool socksChainingProb(double &prob) const;

		bool haveReqMethods() const;
		bool haveReqTypes() const;

		const KerberosWrapSym *kerberosWrap() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
		virtual String msgTypesField() const;
		bool reqRate(double &rate) const;
};

#endif
