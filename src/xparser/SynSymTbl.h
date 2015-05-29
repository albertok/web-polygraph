
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XPARSER_SYNSYMTBL_H
#define POLYGRAPH__XPARSER_SYNSYMTBL_H

// SynSymTbl is a collection of "semantic symbols" with 
// search and accounting capabilities

#include "xstd/Map.h"
#include "xparser/SynSym.h"

class SynSymTbl;

// table element
class SynSymTblItem {
	public:
		SynSymTblItem();
		SynSymTblItem(const String &aType, const String &aName);
		virtual ~SynSymTblItem();

		bool used() const { return theUseCnt > 0; }
		bool defined() const { return theSym != 0; }
		SynSymTbl *ctx() { return theCtx; }
		SynSym *sym() { return theSym; }
		const TokenLoc &loc() const { return theLoc; }
		const String &name() const { return theName; }
		const String &type() const { return theType; }

		void use() { theUseCnt++; }
		void ctx(SynSymTbl *aCtx) { theCtx = aCtx; }
		void sym(SynSym *aSym);
		void loc(const TokenLoc &aLoc) { theLoc = aLoc; }

	protected:
		SynSymTbl *theCtx;  // context
		SynSym *theSym;
		String theName;
		String theType;
		TokenLoc theLoc; // declaration location
		int theUseCnt;
};

class SynSymTbl: public Map<SynSymTblItem*> {
	public:
		SynSymTbl();
		virtual ~SynSymTbl();

		virtual void report(ostream &os, const String &pfx) const;
		void briefReport(ostream &os, const String &pfx) const;

	protected:
		void report(ostream &os, const String &, bool prnUndef) const;

	protected:
		static bool PrnUndef; // report undefined entries
};

#endif
