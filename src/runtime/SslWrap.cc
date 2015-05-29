
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <stdlib.h>
#include "xstd/h/iomanip.h"
#include "xstd/Checksum.h"
#include "xstd/Map.h"

#include "xstd/Assert.h"
#include "xstd/Ssl.h"
#include "xstd/StringIdentifier.h"
#include "xstd/rndDistrs.h"
#include "base/RndPermut.h"
#include "base/AnyToString.h"
#include "pgl/SslWrapSym.h"
#include "runtime/LogComment.h"
#include "runtime/SslWrap.h"

static String TheUnknownFileName = "FILE_NAME_GOES_HERE";

// assembles and runs a key-generation command
// manipulates cached results
class SslCommand {
	public:
		typedef enum { stUnknown, stMustRun, stSuccess, stError } State;

	public:
		static SslCommand Make(ostringstream &os, const char *descr, const SslWrap *aWrap);
		SslCommand(const String aPfx, const char *aDescr, const SslWrap *aWrap);

		String addOutput(const char *argName, const char *argSuffix);
		bool runIfNeeded();

	protected:
		String calcKey(const String &cmd) const;
		String calcFileName(const char *suffix) const;

	private:
		const SslWrap *theWrap;
		String thePrefix;
		String theSuffix;
		String theDescr;
		String theKey;
		State theState;

		typedef Map<State> States; // command has or key : state
		static States TheStates; // cache of known command results
};

SslCommand::States SslCommand::TheStates;

SslWrap::SslWrap(): canShare(false), theProtocolSel(0), theRsaKeySizeSel(0),
	theCipherSel(0), theResumpProb(-1), theSessionCacheSize(-1),
	doVerifyPeerCertificate(true) {

	theServerReqPem = TheUnknownFileName + "-serverreq.pem";
	theServerKeyPem = TheUnknownFileName + "-serverkey.pem";
	theServerCertPem = TheUnknownFileName + "-servercert.pem";
	theServerChainPem = TheUnknownFileName + "-serverchain.pem";
	theCASerialFile = TheUnknownFileName + "-cert.srl";

	// init before we open many sockets or OpenSSL < v0.9.8c may crash
	static bool libInited = false;
	if (!libInited) {
		if (!SslMisc::LibraryInit()) {
			cerr << "error: Polygraph was built without SSL support, "
				<< "can not run SSL workloads" << endl << xexit;
		}
		SslMisc::SeedRng(LclPermut(rndSslSeed));
		Comment(5) << "fyi: SSL library initialized and seeded" << endc;
	}
}

SslWrap::~SslWrap() {
	delete theProtocolSel;
	delete theRsaKeySizeSel;
	delete theCipherSel;
}

void SslWrap::configure(const SslWrapSym &cfg) {
	theRootCertificate = cfg.rootCertificate();
	theSslConfigFile = cfg.sslConfigFile();
	if (!theSslConfigFile) {
		theSslConfigFile = "myssl.conf";
		Comment(1) << cfg.loc() << "warning: using 'myssl.conf' SSL config " <<
			"file. This default is deprecated in favor of an explicit " <<
			"SslWrap.ssl_config_file PGL field." << endc;
	}
	configureProtocols(cfg);
	configureRsaKeySizes(cfg);
	configureCiphers(cfg);
	configureSharing(cfg);

	cfg.sessionResumpt(theResumpProb);
	cfg.sessionCacheSize(theSessionCacheSize);

	if (theSessionCacheSize == 0) {
		Comment(1) << cfg.loc() << "fyi: session cache size of zero means " <<
			"no cache, and not unlimited-size cache as in OpenSSL" << endc;
	}

	if (theResumpProb <= 0 && theSessionCacheSize > 0) {
		Comment(1) << cfg.loc() << "warning: positive session cache size " <<
			"ignored since session resumption is disabled" << endc;
	} else
	if (theResumpProb > 0 && theSessionCacheSize == 0) {
		Comment(1) << cfg.loc() << "warning: positive session resumption " <<
			"probability is ignored since session cache size is zero" << endc;
	}

	if (theResumpProb <= 0)
		theSessionCacheSize = 0;
	else
	if (theSessionCacheSize == 0)
		theResumpProb = 0;

	if (cfg.verifyPeerCertificate(doVerifyPeerCertificate)) {
		if (doVerifyPeerCertificate && !theRootCertificate) {
			Comment << cfg.loc() << "verify_peer_certificate "
				"is set to true but but no root_certificate is "
				"configured" << endc << xexit;
		}
	} else
		doVerifyPeerCertificate = theRootCertificate;
}

void SslWrap::checkProtocol(const SslWrapSym &cfg, const StringIdentifier &sidf, const String &name, const double prob) {
	const int protocol = sidf.lookup(name);
	bool error = false;
	if (protocol <= 0) {
		cerr << cfg.loc() << "error: unknown SSL protocol \"" << name << '"';
		error = true;
	}
	if (!error && prob > 0 && !SslCtx::IsProtocolSupported(protocol)) {
		cerr << cfg.loc() << "error: OpenSSL library does not support \""
			<< name << "\" protocol";
		error = true;
	}
	if (error) {
		cerr << "; supported protocols are:";
		for (TokenIdentifier::Iter i = sidf.iterator(); i; ++i) {
			if (SslCtx::IsProtocolSupported(i.id()))
				cerr << " \"" << i.str() << '"';
		}
		cerr << endl << xexit;
	}
}

void SslWrap::checkProtocols(const SslWrapSym &cfg, const StringIdentifier &sidf) {
	PtrArray<String*> protocols;
	Array<double> probs;
	if (cfg.protocols(protocols, probs)) {
		for (int i = 0; i < protocols.count(); ++i) {
			checkProtocol(cfg, sidf, *protocols[i], probs[i]);
		}
	} else
		checkProtocol(cfg, sidf, "any", 1);
}

void SslWrap::configureProtocols(const SslWrapSym &cfg) {
	static StringIdentifier sidf;
	if (!sidf.count()) {
		sidf.add("SSLv2", SslCtx::SSLv2);
		sidf.add("SSLv3", SslCtx::SSLv3);
		sidf.add("TLSv1", SslCtx::TLSv1);
		sidf.add("any", SslCtx::SSLv23); // all of the above
	}

	checkProtocols(cfg, sidf);

	theProtocolSel = cfg.protocols(sidf);
	if (!theProtocolSel)
		theProtocolSel = new ConstDistr(0, SslCtx::SSLv23);
	theProtocolSel->rndGen(GlbRndGen("ssl_protocols"));
}

void SslWrap::configureRsaKeySizes(const SslWrapSym &cfg) {
	if (cfg.rsaKeySizes(theRsaKeySizes, theRsaKeySizeSel))
		theRsaKeySizeSel->rndGen(GlbRndGen("rsa_key_sizes"));
}

void SslWrap::configureCiphers(const SslWrapSym &cfg) {
	if (cfg.ciphers(theCiphers, theCipherSel))
		theCipherSel->rndGen(GlbRndGen("ssl_ciphers"));
}

// compute output directory and sharing properties
void SslWrap::configureSharing(const SslWrapSym &cfg) {
	const String defaultPath("/tmp/poly-"); // TODO: use $TEMP

	thePath = cfg.sharingGroup();
	if (thePath) {
		if (thePath[0] != '/')
			thePath = defaultPath + thePath;
		canShare = true;
	} else {
		thePath = defaultPath;
		canShare = false;
	}
}

Size SslWrap::selectRsaKeySize() const {
	if (!theRsaKeySizeSel)
		return Size::Bit(1024);
	const int idx = (int)theRsaKeySizeSel->trial();
	return theRsaKeySizes[idx];
}

String SslWrap::selectCipher() const {
	if (!theCipherSel)
		return String("ALL");
	const int idx = (int)theCipherSel->trial();
	return *theCiphers[idx];
}

int SslWrap::sessionCacheSize() const {
	return theSessionCacheSize;
}

double SslWrap::resumpProb() const {
	return theResumpProb;
}

const String &SslWrap::sharingPath() const {
	return thePath;
}

SslCtx *SslWrap::makeClientCtx(const NetAddr &addr) const {
	SslCtx *ctx = makeCtx(addr);

	ctx->setVerify(doVerifyPeerCertificate ? SSL_VERIFY_PEER : SSL_VERIFY_NONE);

	if (!theRootCertificate)
		return ctx;

	// robots need the CA cert to verify server's key
	if (ctx->loadVerifyLocations(theRootCertificate, String()))
		return ctx;

	Comment << "loadVerifyLocations() failed to load root certificate"
		<< endc;

	ReportErrors();
	exit(2);
	return ctx;
}

#ifdef UNUSED_CODE
static
int SslWrap_passwdCb(char *buf, int size, int, void *) {
	strncpy(buf, "password", size);
	return strlen(buf);
}
#endif

SslCtx *SslWrap::makeServerCtx(const NetAddr &addr) const {
	SslCtx *ctx = makeCtx(addr);

	// Use SSL_VERIFY_NONE because Robots do not provide certificates
	// and some man-in-the-middle proxies do not like SSL_VERIFY_PEER.
	// TODO: Honor [explicitly true] verify_peer_certificate.
	ctx->setVerify(SSL_VERIFY_NONE);

	// ctx->setDefaultPasswdCb(&SslWrap_passwdCb); // not needed due to -nodes

	// servers need a private key and the root CA cert
	if (makeSrvPrivateKey(ctx) && makeSrvCert(ctx))
		return ctx;

	exit(2);
	return ctx;
}

SslCtx *SslWrap::makeCtx(const NetAddr &) const {
	const SslCtx::SslProtocol protocol =
		(SslCtx::SslProtocol)theProtocolSel->trial();
	const String cipher = selectCipher();
	return new SslCtx(protocol, cipher);
}

bool SslWrap::makeSrvCert(SslCtx *ctx) const {
	if (!makeServerCert() || !makeServerCertChain())
		return false;

	if (!ctx->useCertificateChainFile(theServerChainPem)) {
		Comment << "error: failed to use certificate chain from "
			<< theServerChainPem << endc;
		ReportErrors();
		return false;
	}

	return true;
}

bool SslWrap::makeServerCert() const {
	// to make a server certificate, we need the CA public key
	// and the CA certificate

	// this command assumes passphrase-less root/CA key
	ostringstream os;
	os << "openssl x509"
		<< " -req"
		<< " -in " << theServerReqPem
		<< " -sha1"
		<< " -extensions usr_cert";

	if (theRootCertificate) {
		os << " -CA " << theRootCertificate
			<< " -CAkey " << theRootCertificate;
	} else {
		os << " -signkey " << theServerKeyPem;
	}

	os << " -CAcreateserial"
		<< ends;

	SslCommand cmd = SslCommand::Make(os, "x509 key generation", this);

	// Use -CAserial option because diskless drones probably won't
	// be able to write the serial file in their current directory.
	// It is not clear whether the serial should be sharing group-specific.
	theCASerialFile = cmd.addOutput(" -CAserial ", "cert.srl");

	theServerCertPem = cmd.addOutput(" -out ", "servercert.pem");
	return cmd.runIfNeeded();
}

bool SslWrap::makeServerCertChain() const {
	// To create a certificate chain, we must concatenate certificates
	ostringstream os;
	os << "cat " << theServerCertPem << ' ' << theServerKeyPem;
	if (theRootCertificate)
		os << ' ' << theRootCertificate;

	SslCommand cmd = SslCommand::Make(os, "certificate chain creation", this);
	theServerChainPem = cmd.addOutput(" > ", "serverchain.pem");
	return cmd.runIfNeeded();
}

bool SslWrap::makeSrvPrivateKey(SslCtx *ctx) const {
	const Size keylen = selectRsaKeySize();
	ostringstream os;
	os << "openssl req"
		<< " -newkey rsa:" << 8*keylen.byte()
		<< " -sha1"
		<< " -nodes"
		<< " -config " << theSslConfigFile
		<< ends;

	SslCommand cmd = SslCommand::Make(os, "server private key generation", this);
	theServerKeyPem = cmd.addOutput(" -keyout ", "serverkey.pem");
	theServerReqPem = cmd.addOutput(" -out ", "serverreq.pem");
	if (!cmd.runIfNeeded())
		return false;
	
	if (!ctx->usePrivateKeyFile(theServerKeyPem)) {
		Comment << "error: failed to use private key from " << 
			theServerKeyPem << endc;
		ReportErrors();
		return false;
	}

	return true;
}

void SslWrap::ReportErrors() {
	const char *fname;
	int line;
	ostream &os = Comment << "SSL error stack:" << endl;
	while (const unsigned long e = SslMisc::ErrGetErrorLine(&fname, &line)) {
		os << "\t" << fname << ":" << line << ": " <<
			SslMisc::ErrErrorString(e) << endl;
	}
	os << endc;
}

SslCommand SslCommand::Make(ostringstream &os, const char *descr,
	const SslWrap *wrap) {
	const String prefix = os.str().c_str();
	streamFreeze(os, false);
	return SslCommand(prefix, descr, wrap);
}

SslCommand::SslCommand(const String aPrefix, const char *aDescr,
	const SslWrap *aWrap): theWrap(aWrap), 
	thePrefix(aPrefix), theDescr(aDescr),
	theState(stUnknown) {
	theKey = calcKey(thePrefix);
}

String SslCommand::addOutput(const char *argName, const char *argSuffix) {
	String argValue = calcFileName(argSuffix);
	theSuffix += argName;
	theSuffix += argValue;
	return argValue;
}

bool SslCommand::runIfNeeded() {
	Assert(theWrap);

	if (theState == stError) // construction error
		return false;

	Assert(theState == stUnknown);

	const String cmd = thePrefix + theSuffix;
	if (cmd.str(TheUnknownFileName)) {
		Comment << "internal error: " << theDescr << " command uses " <<
			"uninitialized file(s): " << cmd << endc;
		theState = stError;
		return false;
	}

	if (!TheStates.find(theKey, theState))
		TheStates.add(theKey, theState);

	// we could check for existing files to reuse files from previous tests
	if (theState == stUnknown) 
		theState = stMustRun;
	else
	if (theState == stSuccess && !theWrap->canShare)
		theState = stMustRun;

	if (theState == stMustRun) {
		Comment(1) << "executing: " << cmd << endc;
		const bool res = ::system(cmd.cstr()) == 0;
		if (!res)
			Comment << "error: " << theDescr << " command failed" << endc;
		theState = res ? stSuccess : stError;
	} else
	if (theState == stSuccess) {
		Comment(1) << "skipping cached: " << cmd << endc;
		Assert(theWrap->canShare);
	} else
	if (theState == stError) {
		Comment(1) << "skipping error: " << cmd << endc;
	} else {
		Comment << "internal error: unknown state " << theState << endc;
		theState = stError;
	}

	*TheStates.valp(theKey) = theState;
	return theState == stSuccess;
}

String SslCommand::calcKey(const String &cmd) const {
	const String path = theWrap->sharingPath();
	Assert(path.len() > 0);
	xstd::ChecksumAlg alg;
	alg.update(cmd.data(), cmd.len());
	alg.update(path.data(), path.len());
	alg.final();
	const String key = PrintToString(alg.sum());
	return key(0, 8);  // first 8 MD5 chars; should be more than enough
}

String SslCommand::calcFileName(const char *suffix) const {
	return theWrap->sharingPath() + theKey + '-' + suffix;
}

