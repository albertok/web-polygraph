
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_ACLSYM_H
#define POLYGRAPH__PGL_ACLSYM_H

#include "pgl/PglRecSym.h"

class String;
class RegExExpr;

// access control "list"
class AclSym: public RecSym {
	public:
		static String TheType;

	public:
		AclSym();
		AclSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		RegExExpr *allow() const;
		RegExExpr *deny() const;
		RegExExpr *rewrite() const;
		bool checkDomestic(double &prob) const;
		bool checkForeign(double &prob) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		RegExExpr *getGroup(const String &name) const;
};

#endif
