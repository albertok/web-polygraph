
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_CREDENTIALSGEN_H
#define POLYGRAPH__PGL_CREDENTIALSGEN_H

#include "base/StringArrayBlocks.h"

// generator of user credentials
class CredentialsGen: public StringArrayBlock {
	public:
		static String TheType;

	public:
		CredentialsGen();
		
		virtual StringArrayBlock *clone() const;

		void configure(int aCount, const String &aNamespace);

		const String &space() const { return theNamespace; };
		virtual int count() const;
		virtual bool find(const Area &member, int &idx) const;

		virtual String item(int idx) const;

	protected:
		virtual bool canMergeSameType(const StringArrayBlock &b) const;
		virtual void mergeSameType(const StringArrayBlock &b);

	protected:
		static String ThePfx;
		static char TheSfx;

		int theId;
		int theCount;
		String theNamespace;

	private:
		static int TheNextId;

		static TypeAnchor TheTypeAnchor;
};

#endif
