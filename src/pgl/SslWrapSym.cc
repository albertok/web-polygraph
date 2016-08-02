
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/String.h"
#include "xstd/TblDistr.h"
#include "pgl/PglBoolSym.h"
#include "pgl/PglRec.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglIntSym.h"
#include "pgl/SslWrapSym.h"



String SslWrapSym::TheType = "SslWrap";

static String strStringArr = "string[]";
static String strProtocols = "protocols";
static String strRootCertificate = "root_certificate";
static String strSizeArr = "size[]";
static String strRsaKeySizes = "rsa_key_sizes";
static String strGenerateCertificates = "generate_certificates";
static String strCertificates = "certificates";
static String strCiphers = "ciphers";
static String strSslCiphers = "ssl-ciphers";	// a global name for the distr
static String strSessionResump = "session_resumption";
static String strSessionCache = "session_cache";
static String strSharingGroup = "sharing_group";
static String strSslConfigFile = "ssl_config_file";
static String strVerifyPeerCertificate = "verify_peer_certificate";
static String strCompression = "compression";


SslWrapSym::SslWrapSym(): RecSym(TheType, new PglRec) {
	theRec->bAdd(strStringArr, strProtocols, 0);
	theRec->bAdd(StringSym::TheType, strRootCertificate, 0);
	theRec->bAdd(StringSym::TheType, strSharingGroup, 0);
	theRec->bAdd(strSizeArr, strRsaKeySizes, 0);
	theRec->bAdd(BoolSym::TheType, strGenerateCertificates, 0);
	theRec->bAdd(strStringArr, strCertificates, 0);
	theRec->bAdd(strStringArr, strCiphers, 0);
	theRec->bAdd(NumSym::TheType, strSessionResump, 0);
	theRec->bAdd(IntSym::TheType, strSessionCache, 0);
	theRec->bAdd(StringSym::TheType, strSslConfigFile, 0);
	theRec->bAdd(BoolSym::TheType, strVerifyPeerCertificate, 0);
	theRec->bAdd(NumSym::TheType, strCompression, 0);
}

SslWrapSym::SslWrapSym(const String &aType, PglRec *aRec): RecSym(aType, aRec) {
}

bool SslWrapSym::isA(const String &type) const {
	return RecSym::isA(type) || type == TheType;
}

SynSym *SslWrapSym::dupe(const String &type) const {
	if (isA(type))
		return new SslWrapSym(this->type(), theRec->clone());
	return RecSym::dupe(type);
}

String SslWrapSym::rootCertificate() const {
        return getString(strRootCertificate);
}

RndDistr *SslWrapSym::protocols(const TokenIdentifier &protocolNames) const {
	return namesToDistr(strProtocols, protocolNames);
}

bool SslWrapSym::protocols(Array<String*> &protos, Array<double> &probs) const {
	return getStrings(strProtocols, protos, &probs);
}

bool SslWrapSym::rsaKeySizes(Array<Size> &sizes, RndDistr *&sel) const {
	SynSymTblItem *wi = 0;
	Assert(theRec->find(strRsaKeySizes, wi));
	if (!wi->sym())
		return false; // undefined

	ArraySym &a = (ArraySym&)wi->sym()->cast(ArraySym::TheType);
	Array<SizeSym*> syms;
	a.exportA(syms);

	sizes.stretch(syms.count());
	for (int i = 0; i < syms.count(); ++i) {
		const Size len = Size::Byte(syms[i]->val().byte());
		const Size minLen = Size::Bit(384);
		if (len < minLen) {
			cerr << loc() << "RSA key size must be at least " << minLen <<
				"; got: " << len << endl;
			exit(1);
		}
		sizes.append(len);
	}

	Array<double> probs;
	a.copyProbs(probs);
	sel = TblDistr::FromDistrTable(type() + "-" + strRsaKeySizes, probs);

	return true;
}

bool SslWrapSym::generateCertificates(bool &set) const {
	return getBool(strGenerateCertificates, set);
}

bool SslWrapSym::certificates(Array<String*> &certificates) const {
	return getStrings(strCertificates, certificates);
}

bool SslWrapSym::ciphers(Array<String*> &ciphers, RndDistr *&selector) const {
	if (ArraySym *as = getArraySym(strCiphers)) {
		selector = as->makeSelector(strSslCiphers);
		Assert(getStrings(strCiphers, ciphers));
		return true;
	}
	return false;
}

bool SslWrapSym::sessionResumpt(double &prob) const {
	return getDouble(strSessionResump, prob);
}

bool SslWrapSym::sessionCacheSize(int &count) const {
	return getInt(strSessionCache, count);
}

String SslWrapSym::sharingGroup() const {
	return getString(strSharingGroup);
}

String SslWrapSym::sslConfigFile() const {
	return getString(strSslConfigFile);
}

bool SslWrapSym::verifyPeerCertificate(bool &set) const {
	return getBool(strVerifyPeerCertificate, set);
}

bool SslWrapSym::compression(double &prob) const {
	return getDouble(strCompression, prob);
}
