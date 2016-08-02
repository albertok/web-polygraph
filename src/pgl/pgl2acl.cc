
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "pgl/pgl.h"

#include <fstream>

#include "xstd/RegEx.h"
#include "xstd/Size.h"
#include "xstd/gadgets.h"
#include "base/opts.h"
#include "base/polyOpts.h"
#include "base/CmdLine.h"
#include "base/RndPermut.h"
#include "pgl/PglPp.h"
#include "pgl/PglCtx.h"

#include "pgl/PglStaticSemx.h"
#include "pgl/PglContainerSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglRegExSym.h"
#include "pgl/AgentSymIter.h"
#include "pgl/AclSym.h"
#include "pgl/RobotSym.h"


class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,    "help",          "list of options"),
			theVersOpt(this,    "version",       "package version info"),
			theCfgName(this,    "config <filename>",  "PGL configuration"),
			theCfgDirs(this,    "cfg_dirs <dirs>",  "directories for PGL #includes"),
			theGlbRngSeed(this, "global_rng_seed <int>","per-test r.n.g. seed", 1)
			{}

		virtual bool validate() const;

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theCfgName;
		StrArrOpt theCfgDirs;
		IntOpt theGlbRngSeed;
};

class Rules;

class RuleCondnItem {
	public:
		RuleCondnItem(RegEx *anExpr = 0, RegExExpr::Oper anOper = RegExExpr::opNone): oper(anOper), expr(anExpr) {}

		bool operator ==(const RuleCondnItem &i) const;
		bool operator !=(const RuleCondnItem &i) const { return !(*this == i); }
		int cmp(const RuleCondnItem &i) const;

		RuleCondnItem negate() const;

		ostream &print(ostream &os) const;

	public:
		RegExExpr::Oper oper;
		RegEx *expr;
};

class RuleCondn;
class RuleCondnHash {
	public:
		RuleCondnHash(const RuleCondn &aBase, int expCount = 0);

		int expCount() const { return theExpCount; }
		int capacity() const { return theIds.capacity(); }

		int idxOf(const RuleCondnItem &i) const; // returns -1 if none

		void add(const RuleCondnItem &i, int idx);

	protected:
		int hash(const RuleCondnItem &i) const;

	protected:
		const RuleCondn &theBase;
		Array<int> theIds;
		int theExpCount;
		int theCount;
};

// array of possibly negated RegExes
class RuleCondn {
	public:
		RuleCondn(int aCapacity = 0);

		int count() const { return theItems.count(); }
		bool has(const RuleCondnItem &h, int offset = 0) const;

		const RuleCondnItem &item(int idx) const { return theItems.item(idx); }

		bool operator ==(const RuleCondn &condn) const;
		bool operator !=(const RuleCondn &condn) const { return !(*this == condn); }
		int cmp(const RuleCondn &c) const;

		bool possible() const;
		bool covers(const RuleCondn &c) const;

		void add(const RuleCondnItem &i);

		void sort();
		void prune();
		void merge(const RuleCondn &c);
		void simplify(Rules &rules, int offset);

		ostream &print(ostream &os) const;

	protected:
		void createHash(int expCount = 8);
		void reHash();

	private:
		Array<RuleCondnItem> theItems;
		RuleCondnHash *theHash;
};

// condition -> action
class AclRule {
	public:
		enum Action { rlUnknown, rlAllow, rlDeny, rlRewrite, rlAny };

	public:
		AclRule(Action anAction, RuleCondn *aCondn);

		Action action() const { return theAction; }
		const RuleCondn &condn() const { return *theCondn; }

		bool operator ==(const AclRule &rule) const;
		bool operator !=(const AclRule &rule) const { return !(*this == rule); }
		int cmp(const AclRule &r) const;
		bool covers(const AclRule &r) const;

		void sort();
		void prune();
		void simplify(Rules &rules, int offset);

		ostream &print(ostream &os) const;

	protected:
		Action theAction;
		RuleCondn *theCondn;
};

// array of rules
class Rules: public Array<AclRule*> {
	public:
		Rules(): Array<AclRule*>(16) {}
		~Rules() { while (count()) delete pop(); }

		const AclRule *covered(const AclRule &r, int excIdx = -1) const;

		//const AclRule *haveOtherAction(const AclRule &rule) const;
		//const AclRule *haveAnyAction(const AclRule &rule) const;

		const AclRule *commit(const AclRule &rule);

		void sort();
		void prune();
		void simplify();

		ostream &print(ostream &os) const;
};

class Normalizer {
	public:
		Normalizer(AclRule::Action anAction, const RegExExpr &expr, Rules &rules);

		void run();

	protected:
		void doNoneOp();
		void doNotOp();
		void doAndOp();
		void doOrOp();

	protected:
		AclRule::Action theAction;
		const RegExExpr &theExpr;
		Rules &theRules;
};


static MyOpts TheOpts;


/* MyOpt */

bool MyOpts::validate() const {
	if (!theCfgName)
		cerr << "must specify PGL configuration file (--config)" << endl;
	else
		return true;
	return false;
}


/* RuleCondnItem */

bool RuleCondnItem::operator ==(const RuleCondnItem &item) const {
	return oper == item.oper && expr->image() == item.expr->image();
}

int RuleCondnItem::cmp(const RuleCondnItem &i) const {
	if (const int operDiff = oper - i.oper)
		return operDiff;

	return expr->image().cmp(i.expr->image());
}

RuleCondnItem RuleCondnItem::negate() const {
	const RegExExpr::Oper o = oper == RegExExpr::opNone ?
			RegExExpr::opNot : RegExExpr::opNone;
	return RuleCondnItem(expr, o);
}

ostream &RuleCondnItem::print(ostream &os) const {
	if (oper == RegExExpr::opNot)
		os << "not ";

	const char *scope = 0;
	if (expr->image().cmp("user_group=", 11) == 0)
		scope = "group";
	else
	if (expr->image().cmp("url=", 4) == 0)
		scope = "url";
	else
	if (expr->image().cmp("url_host=", 9) == 0)
		scope = "server-name";
	else
	if (expr->image().cmp("url_host_ip=", 12) == 0)
		scope = "server-ip";
	else
	if (expr->image().cmp("url_path=", 9) == 0)
		scope = "url";

	os << scope << ' ';

	if ((expr->flags() & RegEx::reExact) == 0)
		os << "matches ";

	os << '"' << expr->pattern() << '"';

	return os;
}


/* RuleCondnHash */

RuleCondnHash::RuleCondnHash(const RuleCondn &aBase, int expCount):
	theBase(aBase), theExpCount(expCount), theCount(0) {
	theIds.resize(expCount*3 + 1);
}

int RuleCondnHash::idxOf(const RuleCondnItem &i) const {
	const int pos = hash(i);
	for (int p = pos, c = 0; theIds[p]; ++p, p %= theIds.count(), ++c) {
		const int idx = theIds[p] - 1;
		if (theBase.item(idx) == i)
			return idx;
		Assert(c < theIds.count());
	}
	return -1;
}

void RuleCondnHash::add(const RuleCondnItem &i, int idx) {
	Assert(theCount < theIds.count());
	Assert(idx >= 0);

	int p = hash(i);
	for (; theIds[p]; ++p, p %= theIds.count()) {
		const int idx = theIds[p] - 1;
		if (theBase.item(idx) == i)
			return; // do not hash duplicates?
	}

	theIds[p] = idx + 1;
	theCount++;
}

inline int StrHash(const String &s) {
	const int step = Max(1, s.len() / 16);
	unsigned int res = 0;
	for (int i = 0; i < s.len(); i += step)
		res = res*33U + s.data()[i];

	return Max(1, abs((int)res));
}

int RuleCondnHash::hash(const RuleCondnItem &i) const {
	// XXX: replace StrHash with String::hash
	return abs(i.oper ^ StrHash(i.expr->image())) % theIds.count();
}


/* RuleCondn */

RuleCondn::RuleCondn(int aCapacity): theItems(aCapacity), theHash(0) {
	if (aCapacity > 1)
		createHash(aCapacity);
}

bool RuleCondn::operator ==(const RuleCondn &c) const {
	if (count() != c.count())
		return false;

	for (int i = 0; i < c.count(); ++i) {
		if (!has(c.item(i)))
			return false;
	}
		
	return true;
}

bool RuleCondn::has(const RuleCondnItem &h, int offset) const {
	if (theHash)
		return theHash->idxOf(h) >= offset;

	for (int i = offset; i < count(); ++i) {
		if (item(i) == h)
			return true;
	}
	return false;
}

bool RuleCondn::possible() const {
	for (int i = 0; i < count(); ++i) {
		if (has(item(i).negate(), i+1))
			return false;
	}
	return true;
}

bool RuleCondn::covers(const RuleCondn &c) const {
	for (int i = 0; i < count(); ++i) {
		if (!c.has(item(i)))
			return false;
	}
	return true;
}

int RuleCondn::cmp(const RuleCondn &c) const {
	if (int cntDiff = count() - c.count())
		return cntDiff;

	Assert(count() == c.count());
	for (int i = 0; i < count(); ++i) {
		if (int itemDiff = item(i).cmp(c.item(i)))
			return itemDiff;
	}

	return 0;
}

static
int cmpRuleCondnItems(const void *ip1, const void *ip2) {
	const RuleCondnItem &i1 = *(const RuleCondnItem*)ip1;
	const RuleCondnItem &i2 = *(const RuleCondnItem*)ip2;
	return (i1.cmp(i2));
}

void RuleCondn::sort() {
	delete theHash;
	theHash = 0;
	qsort(theItems.items(), count(), sizeof(*theItems.items()), &cmpRuleCondnItems);
	createHash(count());
}

void RuleCondn::prune() {
	Array<RuleCondnItem> prunedItems;

	// remove duplicates
	for (int i = 0; i < count(); ++i) {
		if (!has(item(i), i+1))
			prunedItems.append(item(i));
	}

	if (prunedItems.count() != count()) {
		theItems = prunedItems;
		reHash();
	}
}

void RuleCondn::merge(const RuleCondn &c) {
	theItems.stretch(count() + c.count());
	for (int i = 0; i < c.count(); ++i)
		if (!has(c.item(i)))
			add(c.item(i));
}

void RuleCondn::simplify(Rules &rules, int offset) {
	Array<RuleCondnItem> prunedItems;

	for (int i = 0; i < count(); ++i) {
		const RuleCondnItem &ci = item(i);
		const RuleCondnItem notCi = ci.negate();
		bool ejectCandidate = true;
		for (int r = offset; ejectCandidate && r < rules.count(); ++r) {
			ejectCandidate = rules[r]->condn().has(ci) ||
				!rules[r]->condn().has(notCi);
		}
		if (!ejectCandidate)
			prunedItems.append(ci);
	}

	if (prunedItems.count() != count()) {
		theItems = prunedItems;
		reHash();
	}
}

ostream &RuleCondn::print(ostream &os) const {
	if (!count())
		return os << "any";

	for (int i = 0; i < count(); ++i) {
		if (i)
			os << " and ";
		item(i).print(os);
	}
	return os;
}

void RuleCondn::add(const RuleCondnItem &i) {
	if (count() && !theHash)
		createHash();

	if (theHash && count() > 0.75*theHash->capacity())
		reHash();

	if (theHash)
		theHash->add(i, theItems.count());

	theItems.append(i);
}

void RuleCondn::createHash(int expCount) {
	if (theHash && expCount <= theHash->expCount())
		return;

	if (theHash)
		delete theHash;
	theHash = new RuleCondnHash(*this, expCount);

	for (int i = 0; i < count(); ++i)
		theHash->add(item(i), i);
}

void RuleCondn::reHash() {
	delete theHash;
	theHash = 0;
	createHash(count());
}

/* AclRule */

AclRule::AclRule(Action anAction, RuleCondn *aCondn):
	theAction(anAction), theCondn(aCondn) {
}

bool AclRule::operator ==(const AclRule &rule) const {
	return theAction == rule.theAction && (*theCondn) == (*rule.theCondn);
}

bool AclRule::covers(const AclRule &r) const {
	return action() == r.action() && theCondn->covers(*r.theCondn);
}

int AclRule::cmp(const AclRule &r) const {
	if (const int cndDiff = theCondn->cmp(*r.theCondn))
		return cndDiff;

	return action() - r.action();
}

void AclRule::sort() {
	theCondn->sort();
}

void AclRule::prune() {
	theCondn->prune();
}

void AclRule::simplify(Rules &rules, int offset) {
	theCondn->simplify(rules, offset);
}

ostream &AclRule::print(ostream &os) const {
	if (theAction == rlAllow)
		os << "allow ";
	else
	if (theAction == rlDeny)
		os << "deny ";
	else
	if (theAction == rlRewrite)
		os << "rewrite ";
	else
		Assert(false);

	Assert(theCondn);
	theCondn->print(os);
	return os << endl;
}

/* Rules */

#if FUTURE_CODE
// checks both for (A,a) and (A,any)
const AclRule *Rules::cover(const AclRule &rule) const {
	for (int i = 0; i < count(); ++i) {
		if (item(i)->action() != rule.action())
			continue;
		if (item(i).body() == AclRule::AnyCondn)
			return &item(i);
		if (rule.body() == item(i)::body())
			return &item(i);
	}
	return 0;
}

// checks for both (!A,a) and (!A,any)
const AclRule *Rules::coverOtherAction(const AclRule &rule) const {
	for (int i = 0; i < count(); ++i) {
		if (item(i).action() == rule.action())
			continue;
		if (item(i).body() == AclRule::AnyCondn)
			return &item(i);
		if (rule.body() == item(i)::body())
			return &item(i);
	}
	return 0;
}

// checks for (*,a)
const AclRule *Rules::coverAnyAction(const AclRule &rule) const {
	for (int i = 0; i < count(); ++i) {
		if (rule.body() == item(i)::body())
			return &item(i);
	}
	return 0;
}

const AclRule *Rules::commit(const AclRule &rule) {
	append(rule.clone());
	return last();
}
#endif

const AclRule *Rules::covered(const AclRule &r, int excIdx) const {
	for (int i = 0; i < count(); ++i) {
		if (i == excIdx)
			continue;
		if (item(i)->covers(r))
			return item(i);
	}
	return 0;
}

void Rules::prune() {
	// prune individual rules
	{for (int i = 0; i < count(); ++i)
		item(i)->prune();
	}

	// remove duplicates and rules with impossible conditions
	// also remove rules that are covered by more general rules
	{for (int i = 0; i < count();) {
		const AclRule &r = *item(i);
		bool bad = !r.condn().possible() || covered(*item(i), i);
		for (int goodIdx = 0; !bad && goodIdx < i; ++goodIdx) {
			bad = *item(goodIdx) == r;
		}
		if (bad)
			eject(i);
		else
			++i;
	}}
}

static
int cmpRules(const void *rp1, const void *rp2) {
	const AclRule *r1 = *(const AclRule**)rp1;
	const AclRule *r2 = *(const AclRule**)rp2;
	return (r1->cmp(*r2));
}

void Rules::sort() {
	// sort individual rules
	for (int i = 0; i < count(); ++i)
		item(i)->sort();

	qsort(items(), count(), sizeof(*items()), &cmpRules);
}

void Rules::simplify() {
	for (int i = 0; i < count(); ++i)
		item(i)->simplify(*this, i+1);
}

ostream &Rules::print(ostream &os) const {
	for (int i = 0; i < count(); ++i) {
		//cout << setw(4) << setfill('0') << i << setfill(' ') << ' ';
		item(i)->print(cout);
	}
	return os;
}

/* Normalizer */

Normalizer::Normalizer(AclRule::Action anAction, const RegExExpr &anExpr, Rules &aRules):
	theAction(anAction), theExpr(anExpr), theRules(aRules) {
}

void Normalizer::doNoneOp() {
	RuleCondn *condn = new RuleCondn(1);
	condn->add(RuleCondnItem(theExpr.theVal));
	AclRule *rule = new AclRule(theAction, condn);
	theRules.append(rule);
}

void Normalizer::doNotOp() {
	RegExExpr *e = theExpr.theLhs;
	Assert(e);
	switch (e->theOper) {
		case RegExExpr::opNone: {
			RuleCondn *condn = new RuleCondn(1);
			condn->add(RuleCondnItem(e->theVal, RegExExpr::opNot));
			AclRule *rule = new AclRule(theAction, condn);
			theRules.append(rule);
			break;
		}
		case RegExExpr::opNot: {
			Normalizer n(theAction, *e->theLhs, theRules);
			n.run();
			break;
		}
		case RegExExpr::opAnd: {
			RegExExpr notL(e->theLhs, RegExExpr::opNot, 0);
			RegExExpr notR(e->theRhs, RegExExpr::opNot, 0);
			RegExExpr x(&notL, RegExExpr::opOr, &notR);
			Normalizer n(theAction, x, theRules);
			n.run();
			break;
		}
		case RegExExpr::opOr: {
			RegExExpr notL(e->theLhs, RegExExpr::opNot, 0);
			RegExExpr notR(e->theRhs, RegExExpr::opNot, 0);
			RegExExpr x(&notL, RegExExpr::opAnd, &notR);
			Normalizer n(theAction, x, theRules);
			n.run();
			break;
		}
		default:
			Assert(false);
	}
}

void Normalizer::doOrOp() {
	Normalizer lhs(theAction, *theExpr.theLhs, theRules);
	lhs.run();

	Normalizer rhs(theAction, *theExpr.theRhs, theRules);
	rhs.run();
}
		
void Normalizer::doAndOp() {
	Rules lhsRules;
	Normalizer lhs(theAction, *theExpr.theLhs, lhsRules);
	lhs.run();

	Rules rhsRules;
	Normalizer rhs(theAction, *theExpr.theRhs, rhsRules);
	rhs.run();

	//rhsRules.prune();
	//lhsRules.prune();

	theRules.stretch(theRules.count() + lhsRules.count()*rhsRules.count());
	for (int l = 0; l < lhsRules.count(); ++l) {
		for (int r = 0; r < rhsRules.count(); ++r) {
			RuleCondn *condn = new RuleCondn;
			condn->merge(lhsRules[l]->condn());
			condn->merge(rhsRules[r]->condn());
			AclRule *rule = new AclRule(theAction, condn);
			theRules.append(rule);
		}
	}
}

void Normalizer::run() {
	switch (theExpr.theOper) {
		case RegExExpr::opNone: {
			doNoneOp();
			break;
		}
		case RegExExpr::opNot: {
			doNotOp();
			break;
		}
		case RegExExpr::opAnd: {
			doAndOp();
			break;
		}
		case RegExExpr::opOr: {
			doOrOp();
			break;
		}
		default:
			Assert(false);
	}
}

static
void normalizeRules(AclRule::Action action, const RegExExpr &expr, Rules &rules) {
	Normalizer n(action, expr, rules);
	n.run();
}

int main(int argc, char **argv) {
	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate())
		return -1;

	configureStream(cout, 2);
	configureStream(clog, 3);

	GlbPermut().reseed(TheOpts.theGlbRngSeed);

	clog << argv[0] << ": parsing..." << endl;
	TheOpts.theCfgDirs.copy(PglPp::TheDirs);
	PglStaticSemx::Interpret(TheOpts.theCfgName);

	clog << argv[0] << ": collecting rules..." << endl;
	// collect all rules
	Rules rules;
	for (AgentSymIter ai(PglStaticSemx::TheAgentsToUse, RobotSym::TheType, false); ai; ++ai) {
		const RobotSym &robot = (const RobotSym&)*ai.agent();
		if (AclSym *acl = robot.acl()) {
			if (acl->allow())
				normalizeRules(AclRule::rlAllow, *acl->allow(), rules);
			if (acl->deny())
				normalizeRules(AclRule::rlDeny, *acl->deny(), rules);
			if (acl->rewrite())
				normalizeRules(AclRule::rlRewrite, *acl->rewrite(), rules);
		}
	}
	
	// collect all groups
	//for (int i = 0; i < PglSemx::TheUserGroupsToUse; ++i)
	//	TheGroups.append(PglSemx::TheUserGroupsToUse[i]);

	clog << argv[0] << ": pruning " << rules.count() << " rules ..." << endl;
	rules.prune();
	// check for conflicts
	// check for coverage (holes)

	clog << argv[0] << ": sorting " << rules.count() << " rules ..." << endl;
	rules.sort();

	clog << argv[0] << ": simplifying..." << endl;
	rules.simplify(); // prune based on order

	clog << argv[0] << ": printing " << rules.count() << " rules ..." << endl;
	rules.print(cout);

	return 0;
}
