
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_CREDARRSYM_H
#define POLYGRAPH__PGL_CREDARRSYM_H

#include "pgl/PglStringArrayBlockSym.h"
#include "pgl/CredentialsGen.h"

class StringSym;

// a compact representation of an array of user credentials
class CredArrSym: public StringArrayBlockSym {
	public:
		static String TheType;

	public:
		CredArrSym();
		virtual ~CredArrSym();

		virtual bool isA(const String &type) const;

		virtual int count() const;
		virtual bool probsSet() const;
		virtual const SynSym *itemProb(int offset, double &prob) const;

		void configure(int aCount, const String &aNamespace);

		virtual const StringArrayBlock &val() const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &type) const;

	protected:
		CredentialsGen theGen;

		mutable StringSym *theCreds; // cache for itemProb()
};

#endif
