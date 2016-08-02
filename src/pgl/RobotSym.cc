
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "xstd/rndDistrs.h"
#include "xstd/TblDistr.h"
#include "base/RndPermut.h"
#include "pgl/PglBoolSym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglRateSym.h"
#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/AclSym.h"
#include "pgl/ClientBehaviorSym.h"
#include "pgl/ContentSym.h"
#include "pgl/DnsResolverSym.h"
#include "pgl/KerberosWrapSym.h"
#include "pgl/SessionSym.h"
#include "pgl/RangeSym.h"
#include "pgl/RobotSym.h"


const String RobotSym::TheType = "Robot";

static String strAcl = "acl";
static String strAddrArr = "addr[]";
static String strAuth_error = "auth_error";
static String strCredentials = "credentials";
static String strDns_resolver = "dns_resolver";
static String strEmbed_recur = "embed_recur";
static String strForeign_trace = "foreign_trace";
static String strRaw_uri_thrower = "raw_uri_thrower";
static String strIcp_port = "icp_port";
static String strInterests = "interests";
static String strMinimize_new_conn = "minimize_new_conn";
static String strOpen_conn_lmt = "open_conn_lmt";
static String strOrigins = "origins";
static String strPeer_http = "peer_http";
static String strPeer_icp = "peer_icp";
static String strPrivate_cache_cap = "private_cache_cap";
static String strProxies = "proxies";
static String strFtpProxies = "ftp_proxies";
static String strHttpProxies = "http_proxies";
static String strSocksProxies = "socks_proxies";
static String strRecurrence = "recurrence";
static String strSpnegoAuthRatio = "spnego_auth_ratio";
static String strReq_inter_arrival = "req_inter_arrival";
static String strReq_methods = "req_methods";
static String strContainerTags = "container_tags";
static String strReq_rate = "req_rate";
static String strReq_types = "req_types";
static String strSession = "session";
static String strStringArr = "string[]";
static String strTime_distr = "time_distr";
static String strFloat_distr = "float_distr";
static String strSize_distr = "size_distr";
static String strUnique_urls = "unique_urls";
static String strWait_xact_lmt = "wait_xact_lmt";
static String strPipeline_depth = "pipeline_depth";
static String strCookies_keep_lmt = "cookies_keep_lmt";
static String strAcceptContentEncodings = "accept_content_encodings";
static String strRanges = "ranges";
static String strRangeArr = "Range[]";
static String strReqBodyPauseProb = "req_body_pause_prob";
static String strReqBodyPauseStart = "req_body_pause_start";
static String strReqBodyRecurrence = "req_body_recurrence";
static String strContentArr = "Content[]";
static String strPostContents = "post_contents";
static String strPutContents = "put_contents";
static String strUploadContents = "upload_contents";
static String strPassiveFtp = "passive_ftp";
static String strSocksProb = "socks_prob";
static String strSocksChainingProb = "socks_chaining_prob";
static String strKerberosWrap = "kerberos_wrap";

RobotSym::RobotSym(const String &aType): AgentSym(aType) {
	theRec->bAdd(strAddrArr, strProxies, 0);
	theRec->bAdd(strAddrArr, strFtpProxies, 0);
	theRec->bAdd(strAddrArr, strHttpProxies, 0);
	theRec->bAdd(strAddrArr, strSocksProxies, 0);
	theRec->bAdd(strAddrArr, strOrigins, 0);
	theRec->bAdd(RateSym::TheType, strReq_rate, 0);
	theRec->bAdd(strTime_distr, strReq_inter_arrival, 0);
	theRec->bAdd(NumSym::TheType, strRecurrence, 0);
	theRec->bAdd(NumSym::TheType, strSpnegoAuthRatio, 0);
	theRec->bAdd(NumSym::TheType, strEmbed_recur, 0);
	theRec->bAdd(strStringArr, strInterests, 0);
	theRec->bAdd(strStringArr, strReq_types, 0);
	theRec->bAdd(strStringArr, strReq_methods, 0);
	theRec->bAdd(strStringArr, strContainerTags, 0);
	theRec->bAdd(strStringArr, strAcceptContentEncodings, 0);
	theRec->bAdd(IntSym::TheType, strPrivate_cache_cap, 0);
	theRec->bAdd(BoolSym::TheType, strUnique_urls, 0);
	theRec->bAdd(IntSym::TheType, strOpen_conn_lmt, 0);
	theRec->bAdd(IntSym::TheType, strWait_xact_lmt, 0);
	theRec->bAdd(NumSym::TheType, strMinimize_new_conn, 0);
	theRec->bAdd(strStringArr, strCredentials, 0);
	theRec->bAdd(NumSym::TheType, strAuth_error, 0);
	theRec->bAdd(AclSym::TheType, strAcl, new AclSym);
	theRec->bAdd(SessionSym::TheType, strSession, new SessionSym);
	theRec->bAdd(IntSym::TheType, strIcp_port, 0);
	theRec->bAdd(NetAddrSym::TheType, strPeer_http, 0);
	theRec->bAdd(NetAddrSym::TheType, strPeer_icp, 0);
	theRec->bAdd(DnsResolverSym::TheType, strDns_resolver, new DnsResolverSym);
	theRec->bAdd(StringSym::TheType, strForeign_trace, 0);
	theRec->bAdd(StringSym::TheType, strRaw_uri_thrower, 0);
	theRec->bAdd(strFloat_distr, strPipeline_depth, 0);
	theRec->bAdd(strFloat_distr, strCookies_keep_lmt, 0);
	theRec->bAdd(strRangeArr, strRanges, 0);
	theRec->bAdd(NumSym::TheType, strReqBodyPauseProb, 0);
	theRec->bAdd(SizeSym::TheType, strReqBodyPauseStart, 0);
	theRec->bAdd(NumSym::TheType, strReqBodyRecurrence, 0);
	theRec->bAdd(strContentArr, strPostContents, 0);
	theRec->bAdd(strContentArr, strPutContents, 0);
	theRec->bAdd(strContentArr, strUploadContents, 0);
	theRec->bAdd(NumSym::TheType, strPassiveFtp, 0);
	theRec->bAdd(NumSym::TheType, strSocksProb, 0);
	theRec->bAdd(NumSym::TheType, strSocksChainingProb, 0);
	theRec->bAdd(KerberosWrapSym::TheType, strKerberosWrap, 0);
}

RobotSym::RobotSym(const String &aType, PglRec *aRec): AgentSym(aType, aRec) {
}

bool RobotSym::isA(const String &type) const {
	return type == TheType || AgentSym::isA(type);
}

SynSym *RobotSym::dupe(const String &type) const {
	if (type == ClientBehaviorSym::TheType) {
		// cast Robot to ClientBehavior
		ClientBehaviorSym *const cb = new ClientBehaviorSym();
		cb->rec()->copyCommon(*theRec);
		return cb;
	}
	if (isA(type))
		return new RobotSym(this->type(), theRec->clone());
	return AgentSym::dupe(type);
}

bool RobotSym::reqRate(double &rate) const {
	return getRate(strReq_rate, rate);
}

bool RobotSym::reqInterArrival(RndDistr *&iad) const {
	iad = getDistr(strReq_inter_arrival);
	double rr = -1;
	const bool hasRR = reqRate(rr);
	if (hasRR && iad)
		cerr << loc() << ": cannot specify both request rate"
			<< " and inter-arrival distribution" << endl << xexit;

	if (iad)
		return true;

	if (!hasRR)
		return false;

	if (rr <= 0)
		return true; // iad is nil

	// poisson request stream (default) is modeled using an exponential
	// distribution with a mean of a given request rate
	// all clients should share one rng
	static RndGen rng(GlbPermut(rndRobotSymReqInterArrival));
	iad = new ExpDistr(&rng, 1/rr);
	return true;
}

bool RobotSym::recurRatio(double &ratio) const {
	return getDouble(strRecurrence, ratio);
}

bool RobotSym::spnegoRatio(double &ratio) const {
	return getDouble(strSpnegoAuthRatio, ratio);
}

bool RobotSym::embedRecurRatio(double &ratio) const {
	return getDouble(strEmbed_recur, ratio);
}

bool RobotSym::uniqueUrls(bool &set) const {
	return getBool(strUnique_urls, set);
}

bool RobotSym::openConnLimit(int &lmt) const {
	return getInt(strOpen_conn_lmt, lmt);
}

bool RobotSym::waitXactLimit(int &lmt) const {
	return getInt(strWait_xact_lmt, lmt);
}

bool RobotSym::minimizeNewConn(double &prob) const {
	return getDouble(strMinimize_new_conn, prob);
}

SessionSym *RobotSym::session() const {
	const SynSym *ss = getRecSym(strSession);
	return ss ?
		&((SessionSym&)ss->cast(SessionSym::TheType)) : 0;
}

bool RobotSym::authError(double &prob) const {
	return getDouble(strAuth_error, prob);
}

bool RobotSym::icpPort(int &port) const {
	return getInt(strIcp_port, port);
}

NetAddr RobotSym::peerHttp() const {
	return getNetAddr(strPeer_http);
}

NetAddr RobotSym::peerIcp() const {
	return getNetAddr(strPeer_icp);
}

AclSym *RobotSym::acl() const {
	SynSymTblItem *i = 0;
	Assert(theRec->find(strAcl, i));
	return i->sym() ?
		&((AclSym&)i->sym()->cast(AclSym::TheType)) : 0;
}

DnsResolverSym *RobotSym::dnsResolver() const {
	SynSymTblItem *di = 0;
	Assert(theRec->find(strDns_resolver, di));
	return di->sym() ?
		&((DnsResolverSym&)di->sym()->cast(DnsResolverSym::TheType)) : 0;
}

bool RobotSym::privCache(int &capacity) const {
	return getInt(strPrivate_cache_cap, capacity);
}

bool RobotSym::proxies(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strProxies, addrs);
}

bool RobotSym::ftpProxies(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strFtpProxies, addrs);
}

bool RobotSym::httpProxies(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strHttpProxies, addrs);
}

bool RobotSym::socksProxies(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strSocksProxies, addrs);
}

bool RobotSym::origins(Array<NetAddr*> &addrs) const {
	return getNetAddrs(strOrigins, addrs);
}

bool RobotSym::origins(Array<NetAddr*> &addrs, RndDistr *&selector) const {
	SynSymTblItem *oi = 0;
	Assert(theRec->find(strOrigins, oi));
	if (!oi->sym())
		return false; // undefined

	// build address array and selector
	origins(addrs);
	ArraySym &os = (ArraySym&)oi->sym()->cast(ArraySym::TheType);
	selector = os.makeSelector(kind() + "-origins");
	return true;
}

String RobotSym::msgTypesField() const {
	return strReq_types;
}

bool RobotSym::credentials(Array<String*> &creds) const {
	return getStrings(strCredentials, creds);
}

bool RobotSym::containerTags(Array<String*> &tags) const {
	return getStrings(strContainerTags, tags);
}

bool RobotSym::acceptedContentCodings(Array<String*> &codings) const {
	return getStrings(strAcceptContentEncodings, codings);
}


bool RobotSym::interests(Array<StringSym*> &istrs, Array<double> &iprobs) const {
	if (ArraySym *a = getArraySym(strInterests)) {
		a->copyProbs(iprobs);
		a->exportA(istrs);
		return true;
	}
	return false;
}

RndDistr *RobotSym::interests(const TokenIdentifier &interestKinds) const {
	return namesToDistr(strInterests, interestKinds);
}

RndDistr *RobotSym::reqMethods(const TokenIdentifier &reqMethodNames) const {
	return namesToDistr(strReq_methods, reqMethodNames);
}

RndDistr *RobotSym::pipelineDepth() const {
	return getDistr(strPipeline_depth);
}

String RobotSym::foreignTrace() const {
	return getString(strForeign_trace);
}

String RobotSym::rawUriThrower() const {
	return getString(strRaw_uri_thrower);
}

RndDistr *RobotSym::cookiesKeepLimit() const {
	return getDistr(strCookies_keep_lmt);
}

bool RobotSym::ranges(Array<const RangeSym*> &syms, RndDistr *&sel) const {
	if (ArraySym *a = getArraySym(strRanges)) {
		a->exportA(syms);
		Array<double> probs;
		a->copyProbs(probs);
		sel = TblDistr::FromDistrTable(type() + "-" + strRanges, probs);
		return true;
	}
	return false;
}

bool RobotSym::reqBodyPauseProb(double &f) const {
	return getDouble(strReqBodyPauseProb, f);
}

bool RobotSym::reqBodyPauseStart(BigSize &sz) const {
	return getSize(strReqBodyPauseStart, sz);
}

bool RobotSym::reqBodyRecurrence(double &f) const {
	return getDouble(strReqBodyRecurrence, f);
}

bool RobotSym::reqContents(const String &param, Array<ContentSym*> &syms, RndDistr *&sel) const {
	if (ArraySym *a = getArraySym(param)) {
		a->exportA(syms);
		Array<double> probs;
		a->copyProbs(probs);
		sel = TblDistr::FromDistrTable(type() + "-" + strPostContents, probs);
		return true;
	}
	return false;
}

bool RobotSym::passiveFtp(double &prob) const {
	return getDouble(strPassiveFtp, prob);
}

bool RobotSym::socksProb(double &prob) const {
	return getDouble(strSocksProb, prob);
}

bool RobotSym::socksChainingProb(double &prob) const {
	return getDouble(strSocksChainingProb, prob);
}

bool RobotSym::haveReqMethods() const {
	return getArraySym(strReq_methods);
}

bool RobotSym::haveReqTypes() const {
	return getArraySym(strReq_types);
}

const KerberosWrapSym *RobotSym::kerberosWrap() const {
	const SynSym *const s = getRecSym(strKerberosWrap);
	return s ? &((KerberosWrapSym&)s->cast(KerberosWrapSym::TheType)) : 0;
}
