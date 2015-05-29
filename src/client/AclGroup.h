
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_ACLGROUP_H
#define POLYGRAPH__CLIENT_ACLGROUP_H

#include "xstd/Array.h"

class RegExGroup;
class RegExMatchee;
class AclSym;

// a supposedley nonoverlapping and complete group of ACL rules
class AclGroup {
	public:
		AclGroup();
		~AclGroup();

		void configure(const AclSym &cfg);

		operator void*() const;
		bool operator !() const { return !((void*)*this); }

		const RegExGroup *allow() const { return theAllowRule; }
		const RegExGroup *deny() const { return theDenyRule; }
		const RegExGroup *rewrite() const { return theRewriteRule; }

		const char *ruleName(const RegExGroup *rule) const;

		bool needsCheck(bool foreign) const;
		int match(const RegExMatchee &m, Array<RegExGroup*> &matches) const;

	protected:
		RegExGroup *theAllowRule;
		RegExGroup *theDenyRule;
		RegExGroup *theRewriteRule;
		double theCheckDomesticProb;
		double theCheckForeignProb;
};

#endif
