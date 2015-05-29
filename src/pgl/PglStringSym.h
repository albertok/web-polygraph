
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSTRINGSYM_H
#define POLYGRAPH__PGL_PGLSTRINGSYM_H

#include "pgl/PglExprSym.h"

class String;

// a string

class StringSym: public ExpressionSym {
	public:
		static String TheType;

	public:
		StringSym(const String &aVal);

		virtual bool isA(const String &type) const;

		const String &val() const { return theVal; }
		void val(const String &aVal) { theVal = aVal; }

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

	protected:
		String theVal;
};

#endif
