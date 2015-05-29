
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_UNIQIDSYM_H
#define POLYGRAPH__PGL_UNIQIDSYM_H

#include "base/UniqId.h"
#include "pgl/PglExprSym.h"

// uniq identifier; used for things like world id, etc.

class UniqIdSym: public ExpressionSym {
	public:
		static String TheType;

	public:
		UniqIdSym(const UniqId &aVal);

		const UniqId &val() const { return theVal; }
		void val(const UniqId &aVal) { theVal = aVal; }

		virtual bool isA(const String &type) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

	protected:
		UniqId theVal;
};

#endif
