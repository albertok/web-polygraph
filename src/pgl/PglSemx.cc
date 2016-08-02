
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"
#include "pgl/pgl.h"

#include <memory>

#include "xstd/h/sstream.h"
#include "xstd/CommandToBuffer.h"
#include "xstd/Ring.h"
#include "xstd/RegEx.h"
#include "xstd/rndDistrs.h"
#include "xstd/gadgets.h"

#include "base/RndPermut.h"
#include "base/ForeignTrace.h"
#include "base/loadTblDistr.h"
#include "base/AnyToString.h"

#include "xparser/TokenSym.h"
#include "xparser/ParsSym.h"

#include "pgl/PglCtx.h"
#include "pgl/PglSemx.h"
#include "pgl/PglParser.h"

#include "pgl/PglBoolSym.h"
#include "pgl/PglExprSym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglQualifSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglRecSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglListSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglClonerSym.h"
#include "pgl/PglCodeSym.h"
#include "pgl/PglDistrSym.h"
#include "pgl/PglPopDistrSym.h"
#include "pgl/PglRateSym.h"
#include "pgl/PglRegExSym.h"
#include "pgl/PglStrRangeSym.h"
#include "pgl/PglNetAddrParts.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/PglNetAddrRangeSym.h"
#include "pgl/PglUndefOpExprSym.h"
#include "pgl/popDistrs.h"
#include "pgl/pglStrIs.h"

#include "pgl/AclSym.h"
#include "pgl/CacheSym.h"
#include "pgl/DnsCacheSym.h"
#include "pgl/ClientBehaviorSym.h"
#include "pgl/ContentSym.h"
#include "pgl/EveryCodeSym.h"
#include "pgl/CredArrSym.h"
#include "pgl/DumperSym.h"
#include "pgl/GoalSym.h"
#include "pgl/RptmstatSym.h"
#include "pgl/AddrMapSym.h"
#include "pgl/DnsResolverSym.h"
#include "pgl/KerberosWrapSym.h"
#include "pgl/SslWrapSym.h"
#include "pgl/BenchSym.h"
#include "pgl/BenchSideSym.h"
#include "pgl/PolyMix3AsSym.h"
#include "pgl/PolyMix4AsSym.h"
#include "pgl/WebAxe4AsSym.h"
#include "pgl/SrvLb4AsSym.h"
#include "pgl/SpreadAsSym.h"
#include "pgl/IpsToNames.h"
#include "pgl/MimeSym.h"
#include "pgl/ObjLifeCycleSym.h"
#include "pgl/RobotSym.h"
#include "pgl/SocketSym.h"
#include "pgl/PhaseSym.h"
#include "pgl/ProxySym.h"
#include "pgl/PopModelSym.h"
#include "pgl/StatSampleSym.h"
#include "pgl/StatsSampleSym.h"
#include "pgl/ServerSym.h"
#include "pgl/SessionSym.h"
#include "pgl/NetPipeSym.h"
#include "pgl/UniqIdSym.h"
#include "pgl/MembershipMapSym.h"
#include "pgl/SingleRangeSym.h"
#include "pgl/MultiRangeSym.h"
#include "pgl/DynamicNameSym.h"
#include "pgl/ZoneFile.h"
#include "pgl/MimeHeaderSym.h"

static const String strAddrArr = "addr[]";
int PglSemx::TheWorkerId = 1;
String PglSemx::TheWorkerIdStr = "1";


template<class T>
inline
T *Place(T *something, const TokenLoc &loc) {
	if (something && !something->loc())
		something->loc(loc);
	return something;
}

PglSemx::PglSemx(): theCtx(0) {
	Assert(!theCtx);
	theCtx = PglCtx::RootCtx();
	Assert(theCtx);
}

PglSemx::~PglSemx() {
	theCtx = 0;
}

void PglSemx::interpret(const SynSym &s) {
	if (!isRule(s))
		cerr << s.loc() << "code with no effect near " << s << endl << xexit;

	const ParsSym &pgl = (const ParsSym&)s.cast(ParsSym::TheType);

	if (pgl.ruleName() == "Assignment")
		interpAssignment(pgl);
	else
	if (pgl.ruleName() == "Call")
		interpProcCall(pgl);
	else
	if (pgl.ruleName() == "Code")
		interpCode(pgl);
	else
	if (pgl.ruleName() == "CodeInitDecl")
		interpCodeInitDecl(pgl);
	else
	if (pgl.ruleName() == "IfCode")
		interpIfCode(pgl);
	else
	if (pgl.ruleName() == "EveryCode")
		interpEveryCode(pgl);
	else
	if (pgl.ruleName() == "DeclStatement")
		interpDeclStatement(pgl);
	else
	if (pgl.ruleName() == "ExprInitDecl")
		interpExprInitDecl(pgl);
	else
	if (pgl.ruleName() == "ExprStatement")
		interpExprStatement(pgl);
	else
	if (pgl.ruleName() == "PureDecl")
		interpPureDecl(pgl);
	else
	if (pgl.ruleName() == "SimpleStatement")
		interpSimpleStatement(pgl);
	else
	if (pgl.ruleName() == "Statement")
		interpStatement(pgl);
	else
	if (pgl.ruleName() == "StatementSeq")
		interpStatementSeq(pgl);
	else
		cerr << pgl.loc() << pgl.ruleName() <<
			" code with no effect near " << pgl << endl << xexit;
}

ListSym *PglSemx::makeList(const ParsSym &pgl) {
	if (pgl.rhsCount() == 0) {
		// List = .
		return Place(new ListSym(), pgl.loc());
	} else
	if (pgl.rhsCount() == 1) {
		// List = Expression .
		ListSym *s = new ListSym();
		addListItem(*s, pgl.rhsRule(0));
		return Place(s, pgl.loc());
	} else
	if (pgl.rhsCount() == 3) {
		// List = List COMMA Expression .
		ListSym *s = makeList(pgl.rhsRule(0));
		addListItem(*s, pgl.rhsRule(2));
		return s;
	}
	unknownRhs(pgl);
	return 0;
}

void PglSemx::addListItem(ListSym &list, const ParsSym &expr) {
	ExpressionSym *item = makeExpression(expr);
	mustBeDefined(item, "list item", expr.loc());
	list.add(*item);
	delete item;
}

ArraySym *PglSemx::makeArray(const ParsSym &pgl, bool ofNames) {
	if (pgl.rhsCount() == 0) {
		// Array = .
		return Place(new ArraySym(), pgl.loc());
	} else
	if (pgl.rhsCount() == 3 && isToken(pgl.rhs(1), COMMA_TOKEN)) {
		// Array = Array COMMA Expression .
		ArraySym *s = makeArray(pgl.rhsRule(0), ofNames);
		addArrayItem(*s, ofNames, pgl.rhsRule(2));
		return Place(s, pgl.loc());
	} else
	if (pgl.rhsCount() == 5) {
		// Array = Array COMMA Expression COLON Expression .
		ArraySym *s = makeArray(pgl.rhsRule(0), ofNames);
		ExpressionSym *e = makeExpression(pgl.rhsRule(4));
		addArrayItem(*s, ofNames, pgl.rhsRule(2), anyToDouble(*e));
		delete e;
		return s;
	} else
	if (pgl.rhsCount() == 1) {
		// Array = Expression .
		ArraySym *s = new ArraySym();
		addArrayItem(*s, ofNames, pgl.rhsRule(0));
		return Place(s, pgl.loc());
	} else
	if (pgl.rhsCount() == 3 && isToken(pgl.rhs(1), COLON_TOKEN)) {
		// Array = Expression COLON Expression .
		ArraySym *s = new ArraySym();
		ExpressionSym *e = makeExpression(pgl.rhsRule(2));
		addArrayItem(*s, ofNames, pgl.rhsRule(0), anyToDouble(*e));
		delete e;
		return Place(s, pgl.loc());
	}
	unknownRhs(pgl);
	return 0;
}

void PglSemx::addArrayItem(ArraySym &arr, bool ofNames, const ParsSym &expr, double prob) {
	ExpressionSym *item = ofNames ?
		(ExpressionSym*) new StringSym(objName(expr)) :
		(ExpressionSym*) makeExpression(expr);
	mustBeDefined(item, "component of an array", expr.loc());
	arr.add(*item, prob);
	delete item;
}

void PglSemx::interpAssignment(const ParsSym &pgl) {
	if (pgl.rhsCount() == 5 && isToken(pgl.rhs(0), LEFTBRACKET_TOKEN)) {
		// Assignment = LEFTBRACKET Array RIGHTBRACKET ASGN Expression .
		ExpressionSym *e = makeExpression(pgl.rhsRule(4));
		assignToEach(pgl.rhsRule(1), *e);
		delete e;
		return;
	} else
	if (pgl.rhsCount() == 3 && isToken(pgl.rhs(1), ASGN_TOKEN)) {
		// Assignment = ObjName ASGN Expression .
		ExpressionSym *e = makeExpression(pgl.rhsRule(2));
		assignToOne(pgl.rhsRule(0), *e);
		delete e;
		return;
	} else
	if (pgl.rhsCount() == 5 && isRule(pgl.rhs(3), "Code")) {
		// Assignment = ObjName ASGN LEFTBRACE Code RIGHTBRACE .
		assignCode(pgl.rhsRule(0), pgl.rhsRule(3));
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::assignToOne(const ParsSym &name, const ExpressionSym &expr) {
	SynSymTblItem *i = getDescr(objName(name), name.loc());
	assign(i, expr, name.loc());
}

void PglSemx::assignToEach(const ParsSym &lhs, const ExpressionSym &expr) {
	ArraySym *names = makeArray(lhs, true);

	if (expr.isA(ArraySym::TheType)) {
		const ArraySym &arr = (ArraySym&)expr.cast(ArraySym::TheType);

		/* spread RHS across LHS */

		// build a [possibly] random index 
		Array<int> index(arr.count());
		for (int j = 0; j < arr.count(); ++j)
			index.append(j);

		const bool randomize = names->probsSet();
		if (randomize) {
			static RndGen rng(GlbPermut(rndPglSemxAssignment));
			for (int k = 0; k < index.count(); ++k)
				index.swap(k, rng(0,index.count()));
		}

		const int groupCount = arr.count() / names->count();
		if (!randomize && groupCount * names->count() != arr.count()) {
			cerr << names->loc() << "mismatch in the array sizes in non-randomized group assignment: "
				<< arr.count() << " is not divisible by " << names->count() << endl << xexit;
		}

		// get LHS probabilities		
		Array<double> probs(names->count());
		names->copyProbs(probs);

		// assign each LHS item an appropriate size range from RHS
		// ranges are randomized using index built above
		for (int i = 0, idx = 0; i < names->count(); ++i) {
			Assert(names->item(i));
			ArraySym iarr;

			const int addCnt = randomize ?
				(int)xceil(probs[i]*arr.count(), 1) : groupCount;
			for (; iarr.count() < addCnt && idx < index.count(); ++idx) {
				const int pos = index[idx];
				Assert(arr[pos]);
				iarr.add(*arr[pos]); // XXX: we should use ranges if !randomize
			}

			const StringSym &name =
				(const StringSym &)names->item(i)->cast(StringSym::TheType);
			SynSymTblItem *ni = getDescr(name.val(), name.loc());
			assign(ni, iarr, name.loc());
		}
	} else {
		for (int i = 0; i < names->count(); ++i) {
			Assert(names->item(i));
			const StringSym &name =
				(const StringSym &)names->item(i)->cast(StringSym::TheType);
			SynSymTblItem *ni = getDescr(name.val(), name.loc());
			assign(ni, expr, name.loc());
		}
	}
	
	delete names;
}

void PglSemx::assignCode(const ParsSym &name, const ParsSym &code) {
	const String &oname = objName(name);
	SynSymTblItem *i = getDescr(oname, name.loc());
	if (i->type() == "Code") {
		// assign code without interpreting it; the user will interpret
		assign(i, CodeSym(code), name.loc());
	} else {
		// interpret code in a new context; no explicit assignment
		openContext(oname);
		interpCode(code);
		closeContext();
	}
}

ExpressionSym *PglSemx::makeFuncCall(const ParsSym &pgl) {
	if (pgl.rhsCount() == 4 &&
		isRule(pgl.rhs(0), "Name") && isRule(pgl.rhs(2))) {
		// Call = Name LEFTPARENT List RIGHTPARENT .
		const String &cname = pgl.rhsRule(0).rhsToken(0).spelling();
		const ListSym *args = makeList(pgl.rhsRule(2));
		ExpressionSym *res = callFunc(cname, *args);
		delete args;
		return Place(res, pgl.loc());
	}
	unknownRhs(pgl);
	return 0;
}

void PglSemx::interpProcCall(const ParsSym &pgl) {
	if (pgl.rhsCount() == 4 && 
		isRule(pgl.rhs(0), "Name") && isRule(pgl.rhs(2))) {
		// Call = Name LEFTPARENT List RIGHTPARENT .
		const String &cname = pgl.rhsRule(0).rhsToken(0).spelling();
		const ListSym *args = makeList(pgl.rhsRule(2));
		callProc(cname, *args);
		delete args;
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::interpIfCode(const ParsSym &pgl) {
	if (pgl.rhsCount() == 6 && isRule(pgl.rhs(4), "Code")) {
		BoolSym *guard = &(BoolSym&)
			makeExpression(pgl.rhsRule(1))->cast(BoolSym::TheType);
		if (guard->val()) {
			CodeSym *code = 
				Place(new CodeSym(pgl.rhsRule(4)), pgl.rhs(4).loc());
			interpCode(*code);
			delete code;
		}
		delete guard;
		return;
	} else
	if (pgl.rhsCount() == 10 && isRule(pgl.rhs(4), "Code") && isRule(pgl.rhs(8))) {
		BoolSym *guard = &(BoolSym&)
			makeExpression(pgl.rhsRule(1))->cast(BoolSym::TheType);
		const int codePos = guard->val() ? 4 : 8;
		CodeSym *code = 
			Place(new CodeSym(pgl.rhsRule(codePos)), pgl.rhs(codePos).loc());
		interpCode(*code);
		delete code;
		delete guard;
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::interpEveryCode(const ParsSym &pgl) {
	if (pgl.rhsCount() == 6 && isRule(pgl.rhs(4), "Code")) {
		GoalSym *goal = &(GoalSym&)
			makeExpression(pgl.rhsRule(1))->cast(GoalSym::TheType);
		CodeSym *code = 
			Place(new CodeSym(pgl.rhsRule(4)), pgl.rhs(4).loc());
		EveryCodeSym *ecode = 
			Place(new EveryCodeSym(goal, code), pgl.loc());
		execEveryCode(*ecode);
		delete ecode;
		return;
	}
	unknownRhs(pgl);
}

// default implementation complaints and exits
void PglSemx::execEveryCode(const EveryCodeSym &ecode) {
	// XXX: should print interpretation context name
	cerr << ecode.loc() <<
		"error: use of 'every' statement (watchdog code) in static context " <<
		" near " << ecode << endl << xexit;
}

// default implementation complaints and exits
ExpressionSym *PglSemx::callFunc(const String &cname, const ListSym &args) {
	if (cname == IntSym::TheType) {
		checkArgs(cname, 1, args);
		return new IntSym(anyToInt(*args.item(0)));
	} else
	if (cname == NumSym::TheType) {
		checkArgs(cname, 1, args);
		return new NumSym(anyToDouble(*args.item(0)));
	} else
	if (cname == "undef") {
		checkArgs(cname, 0, args);
		return new UndefOpExprSym();
	} else
	if (cname == "count") {
		checkArgs(cname, 1, args);
		const ContainerSym &arr = (const ContainerSym&)
			extractArg(cname, 0, args, ContainerSym::TheType);
		return new IntSym(arr.count());
	} else
	if (cname == "max") {
		return calcExtreme(cname, args, +1);
	} else
	if (cname == "min") {
		return calcExtreme(cname, args, -1);
	} else
	if (cname == "uniqId") {
		checkArgs(cname, 0, args);
		return new UniqIdSym(UniqId::Create());
	} else
	if (cname == "robotAddrs") {
		return calcAgentAddrs(cname, args, &AddrSchemeSym::robots);
	} else
	if (cname == "serverAddrs") {
		return calcAgentAddrs(cname, args, &AddrSchemeSym::servers);
	} else
	if (cname == "proxyAddrs") {
		return calcAgentAddrs(cname, args, &AddrSchemeSym::proxies);
	} else
	if (cname == "ipsToNames") {
		checkArgs(cname, 2, args);
		const ContainerSym &ips = (const ContainerSym&)
			extractArg(cname, 0, args, strAddrArr);
		const StringSym &domain = (const StringSym&)
			extractArg(cname, 1, args, StringSym::TheType);
		return IpsToNames(ips, domain.val());
	} else
	if (cname == "zoneDomains") {
		checkArgs(cname, 2, args);
		const StringSym &fname = (const StringSym&)
			extractArg(cname, 0, args, StringSym::TheType);
		const ContainerSym &ips = (const ContainerSym&)
			extractArg(cname, 1, args, strAddrArr);
		return ZoneDomains(args.loc(), fname.val(), ips);
	} else
	if (cname == "tracedHosts") {
		return tracedHosts(cname, args);
	} else
	if (cname == "clientHostCount") {
		checkArgs(cname, 1, args);
		const BenchSym &bench = (const BenchSym&)
			extractArg(cname, 0, args, BenchSym::TheType);
		int count = -1;
		if (const String err = bench.clientHostCount(count)) {
			cerr << args.loc() << cname << "() call failed: " << 
				err << endl << xexit;
		}
		return new IntSym(count);
	} else
	if (cname == "credentials") {
		checkArgs(cname, 2, args);
		const IntSym &count = (const IntSym&)
			extractArg(cname, 0, args, IntSym::TheType);
		const StringSym &nameSpace = (const StringSym&)
			extractArg(cname, 1, args, StringSym::TheType);
		return genCredentials(cname, count.val(), nameSpace.val(), args.loc());
	} else
	if (cname == "select") {
		checkArgs(cname, 2, args);
		const ContainerSym &items = (const ContainerSym&)
			extractArg(cname, 0, args, ContainerSym::TheType);
		const IntSym &count =(const IntSym&)
			extractArg(cname, 1, args, IntSym::TheType);
		return selectItems(cname, items, count.val(), args.loc());
	} else
	if (cname == "setAddrPort") {
		checkArgs(cname, 2, args);
		const ContainerSym &items = (const ContainerSym&)
			extractArg(cname, 0, args, ContainerSym::TheType);
		const IntSym &port =(const IntSym&)
			extractArg(cname, 1, args, IntSym::TheType);
		return setAddrPort(cname, items, port.val(), args.loc());
	} else
	if (cname == "anyOf") {
		checkArgs(cname, 1, args);
		const ContainerSym &items = (const ContainerSym&)
			extractArg(cname, 0, args, ContainerSym::TheType);
		return orItems(items);
	} else
	if (cname == "dynamicName") {
		checkArgs(cname, 2, args);
		const NetAddrSym &addr = (const NetAddrSym&)
			extractArg(cname, 0, args, NetAddrSym::TheType);
		const NumSym &prob =(const NumSym&)
			extractArg(cname, 1, args, NumSym::TheType);
		return dynamicName(addr, prob);
	} else
	if (cname == "dynamize") {
		checkArgs(cname, 2, args);
		const ContainerSym &items = (const ContainerSym&)
			extractArg(cname, 0, args, ContainerSym::TheType);
		const NumSym &prob =(const NumSym&)
			extractArg(cname, 1, args, NumSym::TheType);
		return dynamize(cname, items, prob);
	} else
	if (cname == "system") {
		checkArgs(cname, 1, args);
		const StringSym &cmd = (const StringSym&)
			extractArg(cname, 0, args, StringSym::TheType);
		if (const stringstream *const sbuf =
			xstd::CommandToBuffer(cmd.val()))
			return new StringSym(String(sbuf->str()));
		cerr << args.loc() << "system() command " << cmd <<
			" probably failed" << endl << xexit;
	} else
	if (cname == "Worker") {
		checkArgs(cname, 0, args);
		return new IntSym(TheWorkerId);
	} else
	if (DistrSym *ds = isDistr(cname, args)) {
		return ds;
	} else
	if (PopDistrSym *pms = isPopDistr(cname, args)) {
		return pms;
	}

	noCall(cname, args);
	return 0;
}

// default implementation complaints and exits
void PglSemx::callProc(const String &cname, const ListSym &args) {
	if (cname == "print") {
		print(cout, args) << endl;
		return;
	}

	noCall(cname, args);
}

// default implementation complaints and exits
void PglSemx::noCall(const String &cname, const ListSym &args) {
	// XXX: should print interpretation context name
	cerr << args.loc() << "error: unknown or out-of-context call: " <<
		cname << "(" << args << ")" << endl << xexit;
}

void PglSemx::interpCode(const ParsSym &pgl) {
	if (pgl.rhsCount() == 1) {
		// Code = StatementSeq .
		interpStatementSeq(pgl.rhsRule(0));
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::interpStatementSeq(const ParsSym &pgl) {
	if (pgl.rhsCount() == 0) {
		// StatementSeq = .
		return; // nothing to be done
	} else
	if (pgl.rhsCount() == 2) {
		// StatementSeq = Statement StatementSeq .
		interpStatement(pgl.rhsRule(0));
		interpStatementSeq(pgl.rhsRule(1));
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::interpStatement(const ParsSym &pgl) {
	if (pgl.rhsCount() == 2 && isToken(pgl.rhs(1), SEMICOLON_TOKEN)) {
		// Statement = SimpleStatement SEMICOLON .
		interpSimpleStatement(pgl.rhsRule(0));
		return;
	} else
	if (pgl.rhsCount() == 1 && isRule(pgl.rhs(0), "IfCode")) {
		// Statement = IfCode .
		interpIfCode(pgl.rhsRule(0));
		return;
	} else
	if (pgl.rhsCount() == 1 && isRule(pgl.rhs(0), "EveryCode")) {
		// Statement = CondCode .
		interpEveryCode(pgl.rhsRule(0));
		return;
	} else
	if (pgl.rhsCount() == 3 && isToken(pgl.rhs(0), LEFTBRACE_TOKEN)) {
		// Statement = LEFTBRACE Code RIGHTBRACE .
		interpCode(pgl.rhsRule(1));
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::interpSimpleStatement(const ParsSym &pgl) {
	if (pgl.rhsCount() == 0) {
		// SimpleStatement = .
		return; // nothing to be done
	} else
	if (pgl.rhsCount() == 1) {
		if (isRule(pgl.rhs(0), "DeclStatement")) {
			// SimpleStatement = DeclStatement .
			interpDeclStatement(pgl.rhsRule(0));
			return;
		} else
		if (isRule(pgl.rhs(0), "ExprStatement")) {
			// SimpleStatement = ExprStatement .
			interpExprStatement(pgl.rhsRule(0));
			return;
		}
	}
	unknownRhs(pgl);
}

void PglSemx::interpDeclStatement(const ParsSym &pgl) {
	if (pgl.rhsCount() == 1 && pgl.rhsRule(0).ruleName() == "CodeInitDecl") {
		// DeclStatement = CodeInitDecl .
		interpCodeInitDecl(pgl.rhsRule(0));
		return;
	} else
	if (pgl.rhsCount() == 1 && pgl.rhsRule(0).ruleName() == "ExprInitDecl") {
		// DeclStatement = ExprInitDecl .
		interpExprInitDecl(pgl.rhsRule(0));
		return;
	} else
	if (pgl.rhsCount() == 1 && pgl.rhsRule(0).ruleName() == "PureDecl") {
		// DeclStatement = PureDecl .
		interpPureDecl(pgl.rhsRule(0));
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::interpPureDecl(const ParsSym &pgl) {
	if (pgl.rhsCount() == 2) {
		// PureDecl = TypeName ObjName .
		declare(typeName(pgl.rhsRule(0)), objName(pgl.rhsRule(1)), pgl.loc());
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::interpExprInitDecl(const ParsSym &pgl) {
	if (pgl.rhsCount() == 4) {
		// ExprInitDecl = TypeName ObjName ASGN Expression .
		declare(typeName(pgl.rhsRule(0)), objName(pgl.rhsRule(1)), pgl.loc());
		ExpressionSym *e = makeExpression(pgl.rhsRule(3));
		assignToOne(pgl.rhsRule(1), *e);
		delete e;
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::interpCodeInitDecl(const ParsSym &pgl) {
	if (pgl.rhsCount() == 6) {
		// CodeInitDecl = TypeName ObjName ASGN LEFTBRACE Code RIGHTBRACE .
		const String name = objName(pgl.rhsRule(1));
		declare(typeName(pgl.rhsRule(0)), name, pgl.loc());
		openContext(name);
		interpCode(pgl.rhsRule(4));
		closeContext();
		return;
	}
	unknownRhs(pgl);
}

void PglSemx::interpExprStatement(const ParsSym &pgl) {
	if (pgl.rhsCount() != 1 || !pgl.rhs(0).isA(ParsSym::TheType))
		unknownRhs(pgl);

	const String ruleName = pgl.rhsRule(0).ruleName();

	if (ruleName == "Assignment") {
		// ExprStatement = Assignment .
		interpAssignment(pgl.rhsRule(0));
		return;
	} else
	if (ruleName == "Call") {
		// ExprStatement = Call .
		interpProcCall(pgl.rhsRule(0));
		return;
	}

	unknownRhs(pgl);
}

ExpressionSym *PglSemx::makeExpression(const ParsSym &pgl) {
	if (pgl.rhsCount() == 1 && isRule(pgl.rhs(0), "Expression")) {
		// extractArg and others may call us with Expression
		if (pgl.rhsRule(0).rhsCount() == 1)
			return makeExpression(pgl.rhsRule(0));
	} else
	if (pgl.rhsCount() == 1 && isToken(pgl.rhs(0), DQW_STR_TOKEN)) {
		// Expression = DQW_STR .
		return makeString(pgl.rhsToken(0));
	} else
	if (pgl.rhsCount() == 1 && isToken(pgl.rhs(0), SQW_STR_TOKEN)) {
		// Expression = SQW_STR .
		return makeQuotedConstant(pgl.rhsToken(0));
	} else
	if (pgl.rhsCount() == 1 && isRule(pgl.rhs(0), "Call")) {
		return makeFuncCall(pgl.rhsRule(0));
	} else
	if (pgl.rhsCount() == 1 && isRule(pgl.rhs(0), "ObjName")) {
		const String name = objName(pgl.rhsRule(0));
		const SynSym &s = getSym(name, pgl.loc());
		return (ExpressionSym*)anyToAny(s, ExpressionSym::TheType);
	} else
	if (pgl.rhsCount() == 1) {
		// Expression = Bool .
		// Expression = Int .
		// Expression = Num .
		// Expression = Qualif .
		// Expression = Size .
		// Expression = Time .
		return makeTypedConst(pgl.rhsRule(0));
	} else
	if (pgl.rhsCount() == 2 && isToken(pgl.rhs(0), ID_TOKEN)) {
		// Expression = ID RE_CUSTOM .
		// Expression = ID RE_DEFAULT .
		return makeRegEx(pgl.rhsToken(0), pgl.rhsToken(1));
	} else
	if (pgl.rhsCount() == 2 && isToken(pgl.rhs(0))) {
		// Expression = MINUS Expression .
		// Expression = NOT Expression .
		// Expression = PLUS Expression .
		const ExpressionSym::Oper op = pgl.rhsToken(0);
		ExpressionSym *exp = makeExpression(pgl.rhsRule(1));
		ExpressionSym *res = exp->unOper(op);
		delete exp;
		return res;
	} else
	if (pgl.rhsCount() == 3 && isToken(pgl.rhs(1))) {
		// Expression = Expression BOOL_AND Expression .
		// Expression = Expression BOOL_OR Expression .
		// Expression = Expression BOOL_XOR Expression .
		// Expression = Expression CLONE Expression .
		// Expression = Expression DIV Expression .
		// Expression = Expression MOD Expression .
		// Expression = Expression MINUS Expression .
		// Expression = Expression MUL Expression .
		// Expression = Expression PLUS Expression .
		// Expression = Expression POWER Expression .
		// Expression = Expression THRU Expression .
		ExpressionSym *exp1 = makeExpression(pgl.rhsRule(0));
		ExpressionSym *exp2 = makeExpression(pgl.rhsRule(2));
		ExpressionSym *res = makeBinExpr(*exp1, pgl.rhsToken(1), *exp2);
		delete exp1;
		delete exp2;
		return res;
	} else
	if (pgl.rhsCount() == 3 && isToken(pgl.rhs(0), LEFTBRACKET_TOKEN)) {
		// Expression = LEFTBRACKET Array RIGHTBRACKET .
		return makeArray(pgl.rhsRule(1));
	} else
	if (pgl.rhsCount() == 3 && isToken(pgl.rhs(0), LEFTPARENT_TOKEN)) {
		// Expression = LEFTPARENT Expression RIGHTPARENT .
		return makeExpression(pgl.rhsRule(1));
	}

	unknownRhs(pgl);
	return 0;
}

ExpressionSym *PglSemx::makeString(const TokenSym &s) {
	Should(s.id() == DQW_STR_TOKEN);
	bool foundRange = false;
	while (const char *p = s.spelling().chr('[')) {
		// check whether range character is escaped
		if (p > s.spelling().cstr() && *(p-1) != '\\') {
			foundRange = true;
			break;
		}
	}
	if (foundRange)
		return makeStringRange(s);
	else
		return makeStringAtom(s);
}

StringSym *PglSemx::makeStringAtom(const TokenSym &s) {
	// get rid of escape characters if any
	String str;
	const char *unproc = s.spelling().cstr();
	while (const char *p = strchr(unproc, '\\')) {
		str.append(unproc, p-unproc);
		unproc = p + 1;
		if (*unproc) {
			str += *unproc;
			++unproc;
		}
	}
	str += unproc;
	return Place(new StringSym(str), s.loc());
}

StrRangeSym *PglSemx::makeStringRange(const TokenSym &s) {
	PglStrRange *r = new PglStrRange;
	if (r->parse(s.spelling())) {
		StrRangeSym *srs = new StrRangeSym;
		srs->range(r);
		return Place(srs, s.loc());
	}
	cerr << s.loc() << "malformed string range constant: \"" <<
		s.spelling() << "\"" << endl << xexit;
	return 0;
}

ExpressionSym *PglSemx::makeQuotedConstant(const TokenSym &s) {
	Should(s.id() == SQW_STR_TOKEN);

	if (s.spelling().str(": "))
		return makeMimeHeader(s);
	const char *p = s.spelling().chr('/');
	if (p && strchr(p+1, '/'))
		return makeTime(s);
	else
	if (s.spelling().chr('-'))
		return makeAddrRange(s);
	else
		return makeAddrAtom(s);

	// not reached
	Assert(false);	
	return 0;
}

NetAddrSym *PglSemx::makeAddrAtom(const TokenSym &s) {
	PglNetAddrParts parts(s.spelling());
	if (parts.error()) {
		cerr << s.loc() << "malformed address constant '" <<
			s.spelling() << "': " << parts.error() << endl << xexit;
		return 0;
	}
	
	if (!parts.single()) {
		cerr << s.loc() << "multiple addresses found where " <<
			"one was expected near '" << s.spelling() << "'" << endl;
		return 0;
	}

	NetAddrSym *addr = new NetAddrSym();
	addr->val(NetAddr(parts.host(), parts.port()));
	if (parts.ifName())
		addr->setIfname(parts.ifName());
	if (parts.subnet() >= 0)
		addr->setSubnet(parts.subnet());
	return addr;
}

NetAddrRangeSym *PglSemx::makeAddrRange(const TokenSym &s) {
	PglNetAddrRange *ar = new PglNetAddrRange;
	if (ar->parse(s.spelling())) {
		NetAddrRangeSym *ars = new NetAddrRangeSym;
		ars->range(ar);
		return Place(ars, s.loc());;
	}

	cerr << s.loc() << "malformed address range constant: '" <<
		s.spelling() << "'" << endl << xexit;
	return 0;
}

ExpressionSym *PglSemx::makeTypedConst(const ParsSym &pgl) {
	if (pgl.ruleName() == "Bool")
		return makeBool(pgl.rhsToken(0));
	else
	if (pgl.ruleName() == "Int")
		return makeInt(pgl.rhsToken(0));
	else
	if (pgl.ruleName() == "Num")
		return makeNum(pgl.rhsToken(0));
	else
	if (pgl.ruleName() == "Time")
		return makeTime(pgl.rhsToken(0));
	else
	if (pgl.ruleName() == "Size")
		return makeSize(pgl.rhsToken(0));
	else
	if (pgl.ruleName() == "Qualif")
		return makeQualif(pgl.rhsToken(0));

	unknownRhs(pgl);
	return 0;
}

BoolSym *PglSemx::makeBool(const TokenSym &s) {
	const bool val = s.id() == BOOL_TRUE_TOKEN;
	// Bool = BOOL_FALSE | BOOL_TRUE .
	return Place(new BoolSym(val), s.loc());
}

IntSym *PglSemx::makeInt(const TokenSym &s) {
	int v;
	Assert(isInt(s.spelling().cstr(), v));
	return Place(new IntSym(v), s.loc());
}

NumSym *PglSemx::makeNum(const TokenSym &s) {
	double v;
	const char *p = 0;
	Assert(isNum(s.spelling().cstr(), v, &p));
	if (p && *p)
		v /= 100;
	return Place(new NumSym(v), s.loc());
}

SizeSym *PglSemx::makeSize(const TokenSym &s) {
	Assert(s.id() == SIZE_TOKEN || s.id() == SIZE_SCALE_TOKEN);
	BigSize v;
	Assert(pglIsSize(s.spelling(), v));
	return Place(new SizeSym(v), s.loc());
}

QualifSym *PglSemx::makeQualif(const TokenSym &s) {
	if (s.spelling() == "lmt")
		return Place(new QualifSym(QualifSym::qfLmt), s.loc());
	if (s.spelling() == "now")
		return Place(new QualifSym(QualifSym::qfNow), s.loc());
	if (s.spelling() == "nmt")
		return Place(new QualifSym(QualifSym::qfNmt), s.loc());
	Assert(false);
	return 0;
}

TimeSym *PglSemx::makeTime(const TokenSym &s) {
	Time v;
	if (s.id() == TIME_TOKEN || s.id() == TIME_SCALE_TOKEN)
		Assert(pglIsRelTime(s.spelling(), v));
	else
	if (!pglIsAbsTime(s.spelling(), v))
		cerr << s.loc() << "malformed time constant: '" << s.spelling() << "'" << endl << xexit;
	return Place(new TimeSym(v), s.loc());
}

MimeHeaderSym *PglSemx::makeMimeHeader(const TokenSym &s) {
	if (MimeHeaderSym *const sym = MimeHeaderSym::Parse(s.spelling()))
		return Place(sym, s.loc());;

	cerr << s.loc() << "malformed MIME header constant: '" <<
		s.spelling() << "'" << endl << xexit;
	return 0;
}

RegExSym *PglSemx::makeRegEx(const TokenSym &scopeName, const TokenSym &reSym) {
	const char *p = reSym.spelling().str("=~");
	if (!p)
		p = reSym.spelling().str("==");
	Assert(p);

	const char del = p[2];
	const char *reBeg = p+3;
	const char *reEnd = reSym.spelling().rchr(del);
	Assert(reBeg && reEnd && reEnd > reBeg);
	const char *options = reEnd + 1;

	const String pattern = reSym.spelling()(reBeg - reSym.spelling().cstr(),
		reEnd - reSym.spelling().cstr());

	const bool exactMatch = reSym.spelling().cmp("==", 2) == 0;

	int reFlags = (scopeName.spelling() == "user_group" && exactMatch) ?
		0 : RegEx::reIgnoreCase; // default depends on scope name
	if (exactMatch)
		reFlags |= RegEx::reExact;
	if (strchr(options, 'i'))
		reFlags |= RegEx::reIgnoreCase;
	if (strchr(options, 'C'))
		reFlags &= ~RegEx::reIgnoreCase;

	const String reStr = scopeName.spelling() + reSym.spelling();
	RegEx *rex = new RegEx;
	rex->configure(reStr, pattern, reFlags);
	if (!*rex)
		cerr << scopeName.loc() << "malformed regular expression: '" << reStr << "'" << endl << xexit;

	return Place(new RegExSym(new RegExExpr(rex)), scopeName.loc());
}

// expr op expr
ExpressionSym *PglSemx::makeBinExpr(const ExpressionSym &exp1, const TokenSym &ops,
	const ExpressionSym &exp2) {

	ExpressionSym::Oper op(ops);

	// check for various exceptions and auto-conversions

	if (op.clone())
		return makeClone(exp1, exp2);

	// bool op
	if (op.boolAny()) {
		if (BoolSym *e1 = (BoolSym*)exp1.clone(BoolSym::TheType)) {
			if (BoolSym *e2 = (BoolSym*)exp2.clone(BoolSym::TheType)) {
				e1->loc(ops.loc());
				e2->loc(ops.loc());
				ExpressionSym *res = e1->bnOper(op, *e2);
				delete e1;
				delete e2;
				return res;
			}
			noCast(exp2, BoolSym::TheType);
		} else
		if (RegExSym *r1 = (RegExSym*)exp1.clone(RegExSym::TheType)) {
			if (RegExSym *r2 = (RegExSym*)exp2.clone(RegExSym::TheType)) {
				r1->loc(ops.loc());
				r2->loc(ops.loc());
				ExpressionSym *res = r1->bnOper(op, *r2);
				delete r1;
				delete r2;
				return res;
			}
			noCast(exp2, RegExSym::TheType);
		}

		noCast(exp1, "bool' or to `re");
	}

	// i op f = f(i) op f
	if (exp1.canBe(IntSym::TheType) && !exp2.canBe(IntSym::TheType) && exp2.canBe(NumSym::TheType)) {
		NumSym *sf = (NumSym*)exp1.clone(NumSym::TheType);
		Assert(sf);
		sf->loc(ops.loc());
		ExpressionSym *res = sf->bnOper(op, exp2);
		delete sf;
		return res;
	}

	/* rates and such: protect from "e/0" since 0 canBe(anything) */
	if (op.div() && !exp2.isA(IntSym::TheType)) {
		if (exp2.canBe(TimeSym::TheType)) {
			// i,f / t = rate
			if (NumSym *f = (NumSym*)exp1.clone(NumSym::TheType)) {
				TimeSym *time = (TimeSym*)exp2.clone(TimeSym::TheType);
				RateSym *rate = new RateSym(f->val(), time->val());
				rate->loc(ops.loc());
				delete time;
				delete f;
				return rate;
			}

			/*
			// size / t = bandwidth
			if (exp1.isA(SizeSym::TheType)) {
				// return new BwidthSym(e1.val(), e2.val());
			}*/
		}

		if (exp2.canBe(RateSym::TheType)) {
			// i,f / rate = t
			if (NumSym *f = (NumSym*)exp1.clone(NumSym::TheType)) {
				RateSym *rate = (RateSym*)exp2.clone(RateSym::TheType);
				TimeSym *time = new TimeSym(Time::Secd(f->val() / rate->val()));
				time->loc(ops.loc());
				delete rate;
				delete f;
				return time;
			}

			/*
			// size / t = bandwidth
			if (exp1.isA(SizeSym::TheType)) {
				// return new BwidthSym(e1.val(), e2.val());
			}*/
		}
	}

	if (op.mult()) {
		// i,f * t,s,r =  t,s,r * i,f
		if (exp1.canBe(NumSym::TheType) &&
			(exp2.canBe(TimeSym::TheType) || exp2.canBe(SizeSym::TheType) || exp2.canBe(RateSym::TheType)))
			return exp2.bnOper(op, exp1);
	}

	// default: exp1 will handle the op
	return exp1.bnOper(op, exp2);
}

ExpressionSym *PglSemx::makeClone(const ExpressionSym &expr, const ExpressionSym &factors) {
	if (IntSym *factori = (IntSym *) factors.clone(IntSym::TheType)) {
		const int factor = factori->val();
		if (factor >= 0) {
			mustBeDefined(&expr, "clone subject");
			ArraySym *a = new ArraySym;
			if (factor > 0)
				a->add(ClonerSym(expr, factori->val()));
			delete factori;
			return Place(a, expr.loc());
		}
		cerr << factors.loc() << "negative cloning factor (" << factori->val() << ")" << endl << xexit;
	}
	cerr << factors.loc() << "cloning factor must be an integer expression" << endl;
	noCast(factors, IntSym::TheType);
	return 0;
}

String PglSemx::typeName(const ParsSym &pgl) const {
	if (isRule(pgl, "TypeName")) {
		if (pgl.rhsCount() == 1 && isToken(pgl.rhs(0), ID_TOKEN)) {
			// TypeName = ID .
			return pgl.rhsToken(0).spelling();
		} else
		if (pgl.rhsCount() == 3 && isToken(pgl.rhs(0), ID_TOKEN)) {
			// TypeName = ID LEFTBRACKET RIGHTBRACKET .
			return pgl.rhsToken(0).spelling() + "[]";
		}
	}
	unknownRhs(pgl);		
	return String();
}

String PglSemx::objName(const ParsSym &pgl) const {
	if (isRule(pgl, "Expression") && pgl.rhsCount() == 1) {
		return objName(pgl.rhsRule(0));
	} else
	if (isRule(pgl, "ObjName") && pgl.rhsCount() == 1 &&
		isToken(pgl.rhs(0), ID_TOKEN)) {
		// ObjName = ID .
		return pgl.rhsToken(0).spelling();
	} else
	if (isRule(pgl, "ObjNameTail") && pgl.rhsCount() == 1 &&
		isRule(pgl.rhs(0), "Name")) {
		// ObjNameTail = Name .
		return pgl.rhsRule(0).rhsToken(0).spelling();
	} else
	if ((isRule(pgl, "ObjName") || isRule(pgl, "ObjNameTail")) &&
		pgl.rhsCount() == 3 && isRule(pgl.rhs(0), "Name")) {
		// ObjName = Name '.' ObjNameTail .
		// ObjNameTail = Name '.' ObjNameTail .
		const String &name = pgl.rhsRule(0).rhsToken(0).spelling();
		return name + '.' + objName(pgl.rhsRule(2));
	}
	unknownRhs(pgl);		
	return String();
}

// opens new context
void PglSemx::openContext(const String &name) {
	theCtx = new PglCtx(name, theCtx);
}

// closes current context, all non-allocated symbols are destroyed
void PglSemx::closeContext() {
	PglCtx *cur = theCtx;
	theCtx = theCtx->parent();
	Assert(theCtx);
	//cur->report(cerr << here << "closing:" << endl);
	delete cur;
}

// finds corresponding description
SynSymTblItem *PglSemx::findDescr(const String &name) {
	SynSymTblItem *i;
	if (theCtx->find(name, i)) {
		Assert(i);
		return i;
	}
	return 0;
}

// finds corresponding description or exits
SynSymTblItem *PglSemx::getDescr(const String &name, const TokenLoc &loc) {
	if (SynSymTblItem *i = findDescr(name))
		return i;

	cerr << loc;
	if (name.chr('.'))
		cerr << "'" << name << "' or its component is not declared" << endl << xexit;
	else
		cerr << "'" << name << "' is not declared" << endl << xexit;
	cerr << xexit;
	return 0;
}

// finds corresponding (defined) symbol or exits
SynSym &PglSemx::getSym(const String &name, const TokenLoc &loc) {
	SynSymTblItem *i = getDescr(name, loc);
	if (!i->sym())
		cerr << loc << "'" << name << "' is not defined" << endl << xexit;
	return *i->sym();
}

SynSymTblItem *PglSemx::declare(const String &type, const String &name, const TokenLoc &loc) {
	if (name.chr('.')) {
		cerr << loc << "declaration of a record member" <<
		endl << xexit;
	}

	SynSymTblItem *oldi;
	if (theCtx->find(name, oldi)) {
		if (oldi->ctx() != theCtx) {
			if (oldi->loc()) {
				cerr << loc << "warning: local '" << name << "' " <<
					"shadows earlier declaration" << endl;
				cerr << oldi->loc() << "warning: this is the shadowed " <<
					"declaration" << endl;
			} else {
				cerr << loc << "warning: local '" << name << "' " <<
					"shadows internal declaration" << endl;
			}
			// not an error, continue
		} else {
			if (oldi->loc()) {
				cerr << loc << "redeclaration of '" << name << "';"
					<< endl;
				cerr << oldi->loc() << "possible location of the previous " <<
					"declaration" << endl;
			} else {
				cerr << loc << "redeclaration of internally "
					"declared '" << name << "';" << endl;
				cerr << xexit;
			}
			cerr << xexit;
		}
	}

	SynSymTblItem *i = new SynSymTblItem(type, name);
	i->loc(loc);
	theCtx->add(name, i);
	setDefault(i);
	//if (i->sym()) cerr << here << i->sym()->type() << " : " << *i->sym() << endl;
	return i;
}

void PglSemx::assign(SynSymTblItem *i, const SynSym &newVal, const SrcLoc &loc) {
	// if (i->sym()) cerr << here << i->name() << " was: (" << i->sym()->type() << ") " << *i->sym() << endl;
	// cerr << here << i->name() << " = (" << newVal.type() << ") " << newVal << endl;

	Assert(i->sym() != &newVal);
	delete i->sym(); // destroy previous value
	i->sym(0);

	if (newVal.isA(UndefOpExprSym::TheType)) // undefine
		return;

	i->sym(newVal.clone(i->type())); // set new one

	if (!i->sym()) {
		cerr << loc << "assignment of incompatible type; "
			<< "no default conversion from '" << newVal.type()
			<< "' to '" << i->type() << "'" << endl << xexit;
	}
}

void PglSemx::mustBeDefined(const ExpressionSym *const expr, const char *const descr, const SrcLoc &defaultLoc) const {
	if (!expr || expr->isA(UndefOpExprSym::TheType)) {
		cerr << (expr ? expr->loc() : defaultLoc) << "undefined "
			<< descr << endl << xexit;
	}
}

void PglSemx::unknownRhs(const ParsSym &pgl) const {
	cerr << pgl.loc() << "internal error: unexpected parsing tree state " <<
		"for rule " << pgl.ruleName() << " near ";
	pgl.print(cerr, String());
	cerr << endl << xabort;
}

SynSym *PglSemx::anyToAny(const SynSym &s, const String &type) const {
	if (SynSym *clone = s.clone(type))
		return clone;
	noCast(s, type);
	return 0;
}

int PglSemx::anyToInt(const SynSym &s) const {
	if (s.isA(NumSym::TheType)) {
		static const ExpressionSym::Oper op = ExpressionSym::Oper::ToInt();
		const NumSym &f = (const NumSym&)s.cast(NumSym::TheType);
		IntSym *i = IntSym::Fit(op, f.val(), s.loc());
		const int res = i->val();
		delete i;
		return res;
	}

	if (s.isA(StringSym::TheType)) {
		const StringSym &ss =
			(const StringSym&)s.cast(StringSym::TheType);
		int res;
		const char *ptr = 0;
		if (isInt(ss.val().cstr(), res, &ptr) && !StrNotSpace(ptr))
			return res;
		failedCast(s, IntSym::TheType, ss.val());
	}

	if (IntSym *i = (IntSym*)s.clone(IntSym::TheType)) { // default casts
		const int res = i->val();
		delete i;
		return res;
	}

	noCast(s, IntSym::TheType);
	return 0;
}


double PglSemx::anyToDouble(const SynSym &s) const {
	if (s.isA(StringSym::TheType)) {
		const StringSym &ss =
			(const StringSym&)s.cast(StringSym::TheType);
		double res;
		const char *ptr = 0;
		if (isNum(ss.val().cstr(), res, &ptr) && !StrNotSpace(ptr))
			return res;
		failedCast(s, NumSym::TheType, ss.val());
	}

	if (NumSym *f = (NumSym*)s.clone(NumSym::TheType)) { // default casts
		const double res = f->val();
		delete f;
		return res;
	}
	noCast(s, NumSym::TheType);
	return 0;
}

void PglSemx::noCast(const SynSym &s, const String &totype) const {
	cerr << s.loc() << "no conversion from '" << s.type()
		<< "' to '" << totype << "'" << endl << xexit;
}

void PglSemx::failedCast(const SynSym &s, const String &totype, const String &str) const {
	cerr << s.loc() << "conversion from '" << s.type() << "' to '" << totype
		<< "' failed near \"" << str << "\"" << endl << xexit;
}

ArraySym *PglSemx::calcAgentAddrs(const String &cname, const ListSym &args, AddrSchemeSym::AddrCalc calc) {
	checkArgs(cname, 2, args);
	const AddrSchemeSym &scheme = (const AddrSchemeSym&)
		extractArg(cname, 0, args, AddrSchemeSym::TheType);
	const BenchSym &bench = (const BenchSym&)
		extractArg(cname, 1, args, BenchSym::TheType);

	String err;
	ArraySym *addrs = (scheme.*calc)(&bench, err);

	if (!addrs) {
		cerr << args.loc() << cname << "() call failed: " << 
			err << endl << xexit;
	}
	return addrs;
}

static
int cmpNetAddrPtrs(const void *p1, const void *p2) {
	const NetAddr *addr1 = *(const NetAddr**)p1;
	const NetAddr *addr2 = *(const NetAddr**)p2;
	if (addr1->sameButPort(*addr2))
		return 0;
	return (addr2->compare(*addr1));
}

ArraySym *PglSemx::tracedHosts(const String &cname, const ListSym &args) {
	checkArgs(cname, 1, args);
	const StringSym &traceName = (const StringSym&)
		extractArg(cname, 0, args, StringSym::TheType);

	Array<NetAddr*> addrs;
	ForeignTrace trace;
	trace.configure(traceName.val());
	if (!trace.gatherHosts(addrs)) {
		cerr << args.loc() << cname << "(" << traceName << ") call failed: " <<
			"could not extract any host addresses" << endl << xexit;
	}

	// set default port numbers if needed
	for (int i = 0; i < addrs.count(); ++i)
		addrs[i]->port(-1);

	qsort(addrs.items(), addrs.count(), sizeof(*addrs.items()), &cmpNetAddrPtrs);

	// note that we are adding from the end, reversing the reversed order
	ArraySym *res = new ArraySym;
	while (addrs.count()) {
		NetAddr *addr = addrs.pop();
		if (!addrs.count() || *addrs.last() != *addr) {
			NetAddrSym s;
			s.val(*addr);
			res->add(s);
		}
		delete addr;
	}

	return res;
}

ContainerSym *PglSemx::genCredentials(const String &cname, int count, const String &nameSpace, const TokenLoc &loc) {
	if (count < 1) {
		cerr << loc << cname << "(" << count << ", ...) "
			<< "call failed: 'count' must be positive" << endl << xexit;
		return 0; // not reached
	}

	CredArrSym *sym = new CredArrSym;
	sym->configure(count, nameSpace);
	return sym;
}

ContainerSym *PglSemx::selectItems(const String &cname, const ContainerSym &items, int goalCount, const TokenLoc &loc) {
	if (goalCount < 1) {
		cerr << loc << cname << "() call failed: "
			<< "'count' must be positive, got: " << goalCount << endl << xexit;
	}

	if (goalCount > items.count()) {
		cerr << loc << "warning: " << cname << "(array, "
			<< goalCount << ") call needs at least " << goalCount 
			<< " array items, but the array has only " << items.count()
			<< "; hence, some items will be repeated" << endl;
	}

	Ring<int> index(items.count());
	while (index.count() < items.count())
		index.enqueue(index.count());

	ArraySym *res = new ArraySym;
	res->reserve(goalCount);
	for (int i = 0; i < goalCount; ++i) {
		static RndGen rng(GlbPermut(rndPglSemxSelectItems));
		if (res->count() % items.count() == 0)
			index.randomize(rng);
		const int idx = index.dequeue();
		res->add(*items.item(idx));
		index.enqueue(idx);
	}
	Assert(res->count() == goalCount);
	return res;
}

ContainerSym *PglSemx::setAddrPort(const String &cname, const ContainerSym &items, int newPort, const TokenLoc &loc) {
	if (newPort > 65536) {
		cerr << loc << cname << "() call failed: "
			<< "'port' cannot exceed 65536, got: " << newPort << endl << xexit;
	}

	ArraySym *res = new ArraySym;
	const int oldCount = items.count();
	res->reserve(oldCount);
	const bool doProbs = items.probsSet();
	for (int i = 0; i < oldCount; ++i) {
		double prob = -1;
		const SynSym *oldItem = doProbs ? // avoid creating 1/oldCount probs
			items.itemProb(i, prob) : items.item(i);
		NetAddrSym *newItem =
			&static_cast<NetAddrSym&>(oldItem->clone()->cast(NetAddrSym::TheType));
		newItem->portVal(newPort);
		res->add(*newItem, prob);
		delete newItem; // array clones so we have to free
	}
	Assert(res->count() == oldCount);
	return res;
}

RegExSym *PglSemx::orItems(const ContainerSym &items) {
	RegExExpr *expr = 0;
	for (int i = 0; i < items.count(); ++i) {
		RegExExpr *item = SymCast(RegExSym, *items.item(i)).val();
		if (expr)
			expr = new RegExExpr(expr, RegExExpr::opOr, item);
		else
			expr = item;
	}
	return new RegExSym(expr);
}

// create a dynamic name from a static address
DynamicNameSym *PglSemx::dynamicName(const NetAddrSym &addr, const NumSym &prob) {
	DynamicNameSym *const name = new DynamicNameSym();
	name->val(addr.val());
	name->setNewProb(prob.val());
	return name;
}

// create dynamic names from static addresses
ContainerSym *PglSemx::dynamize(const String &, const ContainerSym &items, const NumSym &prob) {
	ArraySym *res = new ArraySym;
	const int inCount = items.count();
	res->reserve(inCount);
	const bool doItemProbs = items.probsSet();
	for (int i = 0; i < inCount; ++i) {
		double itemProb = -1;
		const SynSym *item = doItemProbs ? // avoid creating 1/inCount probs
			items.itemProb(i, itemProb) : items.item(i);
		const NetAddrSym &addr =
			reinterpret_cast<const NetAddrSym&>(item->cast(NetAddrSym::TheType));
		DynamicNameSym *const name = dynamicName(addr, prob);
		res->add(*name, itemProb);
		delete name; // array clones so we have to free
	}
	Assert(res->count() == inCount);
	return res;
}

ostream &PglSemx::print(ostream &os, const ListSym &args, const unsigned int skip) {
	os << "script output: ";
	for (int i = skip; i < args.count(); ++i) {
		const SynSym *s = args[i];
		if (s->isA(StringSym::TheType)) // remove quotes
			os << ((const StringSym&)s->cast(StringSym::TheType)).val();
		else
			s->print(os);
	}
	return os;
}

ExpressionSym *PglSemx::calcExtreme(const String &cname, const ListSym &args, int dir) {
	if (!args.count()) {
		cerr << args.loc() << cname << "() function call needs " <<
			"at least one argument" << endl << xexit;
	}

	// find extreme
	const ExpressionSym *result = &(ExpressionSym&)
		args.item(0)->cast(ExpressionSym::TheType);
	const std::auto_ptr<const TokenSym> op(Place(dir > 0 ?
		new TokenSym("<", LT_TOKEN) : new TokenSym(">", GT_TOKEN),
		args.loc()));
	for (int i = 1; i < args.count(); ++i) {
		const ExpressionSym &arg = (ExpressionSym&)
			args.item(i)->cast(ExpressionSym::TheType);
		const std::auto_ptr<const BoolSym> check(&(BoolSym&)
			makeBinExpr(*result, *op, arg)->cast(BoolSym::TheType));
		if (check->val())
			result = &arg;
	}

	return (ExpressionSym*)anyToAny(*result, ExpressionSym::TheType);
}

// check argument type
const ExpressionSym &PglSemx::extractArg(const String &cname, int idx, const ListSym &args, const String &type) {
	Assert(0 <= idx && idx < args.count());
	const SynSym *arg = args.item(idx);
	if (!arg->canBe(type)) {
		cerr << args.loc() << "bad argument " << idx+1 << " in " << cname <<
			" call; expecting type '" << type << "', but got '" <<
			arg->type() << "'" << endl << xexit;
	}
	return (const ExpressionSym&)arg->cast(ExpressionSym::TheType);
}

// check the number of arguments
void PglSemx::checkArgs(const String &cname, int expCount, const ListSym &args) {
	if (args.count() != expCount) {
		cerr << args.loc() << "bad argument count near '" << cname <<
			"' expected " << expCount << " arguments, got " << 
			args.count() << endl << xexit;
	}
}

// convert all arguments to double (including Size and Time)
void PglSemx::argsToDouble(const String &cname, const ListSym &argsIn, Array<double> &outa) {
	Assert(!outa.count());
	for (int i = 0; i < argsIn.count(); ++i) {
		// note that we do not call anyToDouble because it is less permissive
		const ExpressionSym &expr = (const ExpressionSym&)
			extractArg(cname, i, argsIn, ExpressionSym::TheType);
		if (NumSym *n = (NumSym *)expr.clone(NumSym::TheType)) {
			outa.append(n->val());
			delete n;
		} else
		if (TimeSym *t = (TimeSym*) expr.clone(TimeSym::TheType)) {
			outa.append(t->val().secd());
			delete t;
		} else
		if (SizeSym *sz = (SizeSym*) expr.clone(SizeSym::TheType)) {
			outa.append(sz->val().byted());
			delete sz;
		} else {
			cerr << argsIn.loc() << "invalid type '" << expr.type()
				<< "' for argument #" << (i+1)
				<< " of '" << cname << "'" << endl << xexit;
		}
	}
}

// convert all arguments to int
void PglSemx::argsToInt(const String &, const ListSym &argsIn, Array<int> &outa) {
	Assert(!outa.count());
	for (int i = 0; i < argsIn.count(); ++i)
		outa.append(anyToInt(*argsIn.item(i)));
}

bool PglSemx::isToken(const SynSym &s) const {
	return s.isA(TokenSym::TheType);
}

bool PglSemx::isToken(const SynSym &s, int tokenId) const {
	if (s.isA(TokenSym::TheType)) {
		const TokenSym &token = (const TokenSym &)s.cast(TokenSym::TheType);
		return token.id() == tokenId;
	}
	return false;
}

bool PglSemx::isRule(const SynSym &s) const {
	return s.isA(ParsSym::TheType);
}

bool PglSemx::isRule(const SynSym &s, const char *name) const {
	if (s.isA(ParsSym::TheType)) {
		const ParsSym &rule = (const ParsSym &)s.cast(ParsSym::TheType);
		return rule.ruleName() == name;
	}
	return false;
}

DistrSym *PglSemx::isDistr(const String &cname, const ListSym &args) {
	Array<double> dargs;

	// table distribution requires exceptional handling, others are below
	if (cname == "table") {
		// two call formats: table(filename) and table(filename, valueType)
		const bool withTypeArgument = (args.count() == 2);
		checkArgs(cname, withTypeArgument ? 2 : 1, args);

		const StringSym &fName = (const StringSym&)
			extractArg(cname, 0, args, StringSym::TheType);

		String distrType;

		if (withTypeArgument) {
			const StringSym &aType = (const StringSym&)
				extractArg(cname, 1, args, StringSym::TheType);
			distrType = aType.val() + "_distr";
		}
			
		if (RndDistr *d = LoadTblDistr(fName.val(), distrType))
			return new DistrSym(distrType, d);

		cerr << args.loc() << cname << "() failed to load a " <<
			"distribution table from " << fName << xexit;
	}
	
	// by default, guess type of values by the type of the first argument
	const String dType = args.count() ? args[0]->type() + "_distr" : String();

	// XXX: Args2Double can call exit() while is* should just return 0;
	argsToDouble(cname, args, dargs);

	RndGen *gen = new RndGen(GlbPermut(cname.hash(), rndPglSemxIsDistr));

	if (cname == "const") {
		checkArgs(cname, 1, args);
		return new DistrSym(dType, new ConstDistr(gen, dargs[0]));
	} else
	if (cname == "unif") {
		checkArgs(cname, 2, args);
		return new DistrSym(dType, new UnifDistr(gen, dargs[0], dargs[1]));
	} else
	if (cname == "exp") {
		checkArgs(cname, 1, args);
		return new DistrSym(dType, new ExpDistr(gen, dargs[0]));
	} else
	if (cname == "norm") {
		checkArgs(cname, 2, args);
		return new DistrSym(dType, new NormDistr(gen, dargs[0], dargs[1]));
	} else
	if (cname == "logn") {
		checkArgs(cname, 2, args);
		return new DistrSym(dType, LognDistr::ViaMean(gen, dargs[0], dargs[1]));
	} else
	if (cname == "zipf") {
		checkArgs(cname, 1, args);
		Array<int> iargs;
		argsToInt(cname, args, iargs);
		return new DistrSym(dType, new ZipfDistr(gen, iargs[0]));
	} else
	if (cname == "seq") {
		checkArgs(cname, 1, args);
		Array<int> iargs;
		argsToInt(cname, args, iargs);
		return new DistrSym(dType, new SeqDistr(gen, iargs[0]));
	} else {
		delete gen;
		return 0;
	}
}

PopDistrSym *PglSemx::isPopDistr(const String &cname, const ListSym &args) {
	const String dType = PopDistrSym::TheType;
	Array<double> dargs;
	argsToDouble(cname, args, dargs);

	if (cname == "popUnif") {
		checkArgs(cname, 0, args);
		return new PopDistrSym(dType, new UnifPopDistr);
	} else
	if (cname == "popZipf") {
		checkArgs(cname, 1, args);
		return new PopDistrSym(dType, new ZipfPopDistr(dargs[0]));
	}

	return 0;
}

// note: also checks if the type is a known type!
void PglSemx::setDefault(SynSymTblItem *i) {

	if (!knownType(i->type()))
		cerr << i->loc() << "unknown type '" << i->type() << "'" << endl << xexit;

	if (i->type() == AclSym::TheType)
		i->sym(new AclSym);
	else
	if (i->type() == DumperSym::TheType)
		i->sym(new DumperSym);
	else
	if (i->type() == SocketSym::TheType)
		i->sym(new SocketSym);
	else
	if (i->type() == GoalSym::TheType)
		i->sym(new GoalSym);
	else
	if (i->type() == RptmstatSym::TheType)
		i->sym(new RptmstatSym);
	else
	if (i->type() == DnsResolverSym::TheType)
		i->sym(new DnsResolverSym);
	else
	if (i->type() == AddrMapSym::TheType)
		i->sym(new AddrMapSym);
	else
	if (i->type() == KerberosWrapSym::TheType)
		i->sym(new KerberosWrapSym);
	else
	if (i->type() == SslWrapSym::TheType)
		i->sym(new SslWrapSym);
	else
	if (i->type() == ServerSym::TheType)
		i->sym(new ServerSym);
	else
	if (i->type() == RobotSym::TheType)
		i->sym(new RobotSym);
	else
	if (i->type() == ProxySym::TheType)
		i->sym(new ProxySym);
	else
	if (i->type() == PopModelSym::TheType)
		i->sym(new PopModelSym);
	else
	if (i->type() == BenchSym::TheType)
		i->sym(new BenchSym);
	else
	if (i->type() == BenchSideSym::TheType)
		i->sym(new BenchSideSym);
	else
	if (i->type() == "PolyMix3As")
		i->sym(new PolyMix3AsSym);
	else
	if (i->type() == "PolyMix4As")
		i->sym(new PolyMix4AsSym);
	else
	if (i->type() == "SrvLb4As")
		i->sym(new SrvLb4AsSym);
	else
	if (i->type() == "WebAxe4As")
		i->sym(new WebAxe4AsSym);
	else
	if (i->type() == "SpreadAs")
		i->sym(new SpreadAsSym);
	else
	if (i->type() == CacheSym::TheType)
		i->sym(new CacheSym);
	else
	if (i->type() == DnsCacheSym::TheType)
		i->sym(new DnsCacheSym);
	else
	if (i->type() == MimeSym::TheType)
		i->sym(new MimeSym);
	else
	if (i->type() == ObjLifeCycleSym::TheType)
		i->sym(new ObjLifeCycleSym);
	else
	if (i->type() == PhaseSym::TheType)
		i->sym(new PhaseSym);
	else
	if (i->type() == SessionSym::TheType)
		i->sym(new SessionSym);
	else
	if (i->type() == StatSampleSym::TheType)
		i->sym(new StatSampleSym);
	else
	if (i->type() == StatsSampleSym::TheType)
		i->sym(new StatsSampleSym);
	else
	if (i->type() == ContentSym::TheType)
		i->sym(new ContentSym);
	else
	if (i->type() == NetPipeSym::TheType)
		i->sym(new NetPipeSym);
	else
	if (i->type() == MembershipMapSym::TheType)
		i->sym(new MembershipMapSym);
	else
	if (i->type() == EveryCodeSym::TheType)
		i->sym(new EveryCodeSym);
	else
	if (i->type() == SingleRangeSym::TheType)
		i->sym(new SingleRangeSym);
	else
	if (i->type() == MultiRangeSym::TheType)
		i->sym(new MultiRangeSym);
	else
	if (i->type() == MimeHeaderSym::TheType)
		i->sym(new MimeHeaderSym);
	else
	if (i->type() == ClientBehaviorSym::TheType)
		i->sym(new ClientBehaviorSym);
	// else no default value
}

// checks if the type is a known type
bool PglSemx::knownType(const String &type) {
	static PtrArray<String*> knownTypes;
	if (!knownTypes.count()) {
		// these are all known integral types
		// do not use String[], as it leads to an internal gcc 2.7.2.3 bug
		static const char *kts[] = {
			BoolSym::TheType.cstr(), IntSym::TheType.cstr(),
			NumSym::TheType.cstr(), TimeSym::TheType.cstr(),
			SizeSym::TheType.cstr(), RateSym::TheType.cstr(),
			BwidthSym::TheType.cstr(), NetAddrSym::TheType.cstr(),
			StringSym::TheType.cstr(), UniqIdSym::TheType.cstr(),
			RegExSym::TheType.cstr(), ServerSym::TheType.cstr(),
			RobotSym::TheType.cstr(), ProxySym::TheType.cstr(),
			PopDistrSym::TheType.cstr(), PopModelSym::TheType.cstr(),
			CacheSym::TheType.cstr(), BenchSym::TheType.cstr(),
			BenchSideSym::TheType.cstr(), PolyMix3AsSym::TheType.cstr(),
			PolyMix4AsSym::TheType.cstr(), SrvLb4AsSym::TheType.cstr(),
			WebAxe4AsSym::TheType.cstr(), SpreadAsSym::TheType.cstr(),
			RptmstatSym::TheType.cstr(),
			DnsResolverSym::TheType.cstr(), AddrMapSym::TheType.cstr(),
			EveryCodeSym::TheType.cstr(), 
			GoalSym::TheType.cstr(),
			KerberosWrapSym::TheType.cstr(), SslWrapSym::TheType.cstr(),
			MimeSym::TheType.cstr(), ObjLifeCycleSym::TheType.cstr(),
			ContentSym::TheType.cstr(), PhaseSym::TheType.cstr(),
			StatSampleSym::TheType.cstr(),
			StatsSampleSym::TheType.cstr(), SessionSym::TheType.cstr(),
			NetPipeSym::TheType.cstr(), MembershipMapSym::TheType.cstr(),
			AclSym::TheType.cstr(), DynamicNameSym::TheType.cstr(),SingleRangeSym::TheType.cstr(),
			SingleRangeSym::TheType.cstr(), MultiRangeSym::TheType.cstr(),
			MimeHeaderSym::TheType.cstr(),
			ClientBehaviorSym::TheType.cstr(),
			DnsCacheSym::TheType.cstr(),
			0 // eof
		};
		for (int i = 0; kts[i]; ++i)
			knownTypes.append(new String(kts[i]));
	}

	for (int i = 0; i < knownTypes.count(); ++i) {
		const String &kt = *knownTypes[i];
		if (type == kt)
			return true;
		if (type == kt + "_distr") // distributions
			return true;
		if (type == kt + "[]")     // arrays
			return true;
		if (type == kt + "_distr[]") // arrays of distributions
			return true;
	}

	return false;
}

void PglSemx::WorkerId(int wokerId) {
	TheWorkerId = wokerId;
	TheWorkerIdStr = AnyToString(TheWorkerId);
}

const char* PglSemx::WorkerIdStr() {
	return TheWorkerIdStr.cstr();
}
