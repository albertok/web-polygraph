
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLCTX_H
#define POLYGRAPH__PGL_PGLCTX_H

#include "xparser/SynSymTbl.h"

// holds current parsing context
// contexts can be nested
// contexts may be "named" (equivalent to "using" clause in Pascal)
class PglCtx: public SynSymTbl {
	public:
		static PglCtx *RootCtx() { return TheRootCtx; }

	public:
		PglCtx(const String &aName, PglCtx *aParent = 0);
		virtual ~PglCtx();

		PglCtx *parent() { return theParent; }
		const String &name() const { return theName; }
		int nestDepth() const;

		virtual SynSymTblItem **valp(const String &key);

		virtual void report(ostream &os, const String &) const;

	protected:
		// recursive search
		SynSymTblItem **valpR(const String &key);
		// search for a.b
		SynSymTblItem **memberp(const String &name);

		virtual void noteAdd(int idx);
		virtual void noteDel(int idx);

	protected:
		static PglCtx *TheRootCtx; // root context

	protected:
		String theName;
		PglCtx *theParent; // null for root context
};

#endif
