
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_SSLWRAPSYM_H
#define POLYGRAPH__PGL_SSLWRAPSYM_H

#include "xstd/Array.h"
#include "xstd/Size.h"
#include "pgl/PglRecSym.h"


// a set of addresses that talk SSL and parameters for SSL protocol
class SslWrapSym: public RecSym {
	public:
		static String TheType;

	public:
		SslWrapSym();
		SslWrapSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		String rootCertificate() const;
		String sharingGroup() const;
		bool protocols(Array<String*> &protos, Array<double> &probs) const;
		RndDistr *protocols(const TokenIdentifier &protocolNames) const;
		bool rsaKeySizes(Array<Size>&, RndDistr *&) const;
		bool generateCertificates(bool &set) const;
		bool certificates(Array<String*> &) const;
		bool ciphers(Array<String*> &, RndDistr *&) const;
		bool sessionResumpt(double &prob) const;
		bool sessionCacheSize(int &count) const;
		String sslConfigFile() const;
		bool verifyPeerCertificate(bool &set) const;
		bool compression(double &prob) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
