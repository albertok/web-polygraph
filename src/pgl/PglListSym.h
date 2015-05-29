
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLLISTSYM_H
#define POLYGRAPH__PGL_PGLLISTSYM_H

#include "xstd/Array.h"
#include "pgl/PglExprSym.h"

// list of symbols of any type
class ListSym: protected Array<SynSym*>, public ExpressionSym {
	public:
		static const String TheType;

	public:
		ListSym();
		ListSym(const String &aType);
		ListSym(const ListSym &aList);
		virtual ~ListSym();

		virtual bool isA(const String &type) const;

		int count() const { return Array<SynSym*>::count(); }
		const SynSym *operator [](int idx) const { return item(idx); }
		const SynSym *item(int idx) const { return Array<SynSym*>::item(idx); }

		void add(const SynSym &s);
		void clear();

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		bool absorb(const ListSym &arr);
		void doAdd(SynSym *sclone);

		virtual void addFailure(const SynSym &s);
		virtual bool cadd(const SynSym &s);
};

#endif
