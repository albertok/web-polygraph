
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_SRCLOC_H
#define POLYGRAPH__XSTD_SRCLOC_H

#include "xstd/h/iosfwd.h"
#include "xstd/String.h"

#define Here SrcLoc(__FILE__, __LINE__)


// source location holder
class SrcLoc {
	public:
		SrcLoc(const String &aFname = 0, int aLineNo = 1);

		int lineno() const { return theLineNo; }
		const String &fname() const { return theFName; }

		operator void*() const { return theFName.len() ? (void*)-1 : 0; }
		SrcLoc &operator ++() { ++theLineNo; return *this; }
		bool sameLine(const SrcLoc &t) const;

		ostream &print(ostream &os) const;

	protected:
		String theFName;
		int theLineNo;
};

inline ostream &operator <<(ostream &os, const SrcLoc &l) { return l.print(os); }

#endif
