
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLUNDEFOPEXPRSYM_H
#define POLYGRAPH__PGL_PGLUNDEFOPEXPRSYM_H

#include "pgl/PglExprSym.h"

// "undefined" or "null" expression -- undefines LHS value
class UndefOpExprSym: public ExpressionSym {
	public:
		static const String TheType;

	public:
		UndefOpExprSym();

		virtual bool isA(const String &type) const;

	protected:
		virtual SynSym *dupe(const String &) const;
};

#endif
