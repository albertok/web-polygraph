
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_SSLWRAP_H
#define POLYGRAPH__RUNTIME_SSLWRAP_H

#include "xstd/h/iosfwd.h"
#include "xstd/Size.h"
#include "xstd/String.h"
#include "xstd/Array.h"

class NetAddr;
class RndDistr;
class SslCtx;
class SslWrapSym;
class StringIdentifier;

// configuration and high-level logic for producing agent's SslCtx
// may be shared among many agents (see SslWraps)
class SslWrap {
	public:
		static void ReportErrors();

	public:
		SslWrap();
		~SslWrap();

		void configure(const SslWrapSym &cfg);
		SslCtx *makeClientCtx(const NetAddr &addr) const;
		SslCtx *makeServerCtx(const NetAddr &addr) const;

		int sessionCacheSize() const;
		double resumpProb() const;
		const String &sharingPath() const;
		bool canShare;

	protected:
		void checkProtocol(const SslWrapSym &cfg, const StringIdentifier &sidf, const String &name, const double prob);
		void checkProtocols(const SslWrapSym &cfg, const StringIdentifier &sidf);
		void configureProtocols(const SslWrapSym &cfg);
		void configureRsaKeySizes(const SslWrapSym &cfg);
		void configureCiphers(const SslWrapSym &cfg);
		void configureSharing(const SslWrapSym &cfg);

		SslCtx *makeCtx(const NetAddr &addr) const;
		bool makeSrvCert(SslCtx *ctx) const;
		bool makeSrvPrivateKey(SslCtx *ctx) const;
		bool makeServerCert() const;
		bool makeServerCertChain() const;
		Size selectRsaKeySize() const;
		String selectCipher() const;
		String needParam(const SslWrapSym &sym, String value, const char *pname) const;

	private:
		String theSharingGroup;
		String theRootCertificate;
		String theSslConfigFile;

		mutable String theServerReqPem;
		mutable String theServerKeyPem;
		mutable String theServerCertPem;
		mutable String theServerChainPem;
		mutable String theCASerialFile;
		String thePath;

		RndDistr *theProtocolSel;
		RndDistr *theRsaKeySizeSel;
		Array<Size> theRsaKeySizes;
		RndDistr *theCipherSel;
		Array<String*> theCiphers;
		double theResumpProb;
		int theSessionCacheSize;

		bool doVerifyPeerCertificate;
};

#endif
