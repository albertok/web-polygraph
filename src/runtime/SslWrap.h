
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
		static std::ostream &PrintErrors(std::ostream &os);

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
		void configureCertificates(const SslWrapSym &cfg);
		void configureCiphers(const SslWrapSym &cfg);
		void configureSharing(const SslWrapSym &cfg);
		void configureCompression(const SslWrapSym &cfg);

		SslCtx *makeCtx(const bool isServer) const;
		bool generateCert(const String &kind) const;
		bool makePrivateKey(const String &kind) const;
		bool makeCert(const String &kind) const;
		bool makeCertChain(const String &kind) const;
		Size selectRsaKeySize() const;
		String selectCipher() const;
		bool disableCompression() const;

	private:
		String theSharingGroup;
		String theRootCertificate;
		String theSslConfigFile;

		mutable String theReqPem;
		mutable String theKeyPem;
		mutable String theCertPem;
		mutable String theChainPem;
		mutable String theCASerialFile;
		String thePath;

		RndDistr *theProtocolSel;
		RndDistr *theRsaKeySizeSel;
		Array<Size> theRsaKeySizes;
		Array<String*> theCertificates;
		mutable int theCertificatesIdx;
		RndDistr *theCipherSel;
		Array<String*> theCiphers;
		double theResumpProb;
		int theSessionCacheSize;
		double theCompression;

		bool doVerifyPeerCertificate;
		bool doGenerateCertificates;
		bool isGenerageCertificatesSet;
};

#endif
