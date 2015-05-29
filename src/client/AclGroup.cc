
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Rnd.h"
#include "pgl/AclSym.h"
#include "client/RegExGroup.h"
#include "client/AclGroup.h"


AclGroup::AclGroup(): theAllowRule(0), theDenyRule(0), theRewriteRule(0),
	theCheckDomesticProb(1.0), theCheckForeignProb(1.0) {
}

AclGroup::~AclGroup() {
	// we do not delete rules since we do not allocate them
}

void AclGroup::configure(const AclSym &cfg) {
	theAllowRule = RegExGroup::FromExpr(cfg.allow());
	theDenyRule = RegExGroup::FromExpr(cfg.deny());
	theRewriteRule = RegExGroup::FromExpr(cfg.rewrite());
	cfg.checkDomestic(theCheckDomesticProb);
	cfg.checkForeign(theCheckForeignProb);
}

AclGroup::operator void*() const {
	if (!theAllowRule && !theDenyRule && !theRewriteRule)
		return 0;
	if (theCheckDomesticProb <= 0 && theCheckForeignProb <= 0)
		return 0;
	return (void*)-1;
}

// XXX: use String
const char *AclGroup::ruleName(const RegExGroup *rule) const {
	if (rule == theAllowRule)
		return "allow";
	if (rule == theDenyRule)
		return "deny";
	if (rule == theRewriteRule)
		return "rewrite";
	Should(false);
	return "unknown";
}

bool AclGroup::needsCheck(bool foreign) const {
	static RndGen rng;
	if (foreign) {
		if (!rng.event(theCheckForeignProb))
			return false;
	} else {
		if (!rng.event(theCheckDomesticProb))
			return false;
	}
	return true;
}

int AclGroup::match(const RegExMatchee &m, Array<RegExGroup*> &matches) const {
	// since there is no order for allow/deny/rewrite rules,
	// they must be exclusive and we must check all of the rules
	if (theAllowRule && theAllowRule->match(m))
		matches.append(theAllowRule);
	if (theDenyRule && theDenyRule->match(m))
		matches.append(theDenyRule);
	if (theRewriteRule && theRewriteRule->match(m))
		matches.append(theRewriteRule);

	return matches.count();
}
