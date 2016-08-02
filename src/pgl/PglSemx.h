
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSEMX_H
#define POLYGRAPH__PGL_PGLSEMX_H

#include "xstd/Array.h"
#include "xstd/h/iosfwd.h"
#include "pgl/AddrSchemeSym.h"

class SynSymTblItem;
class TokenSym;
class ParsSym;
class PglCtx;
class IfCodeSym;
class EveryCodeSym;
class ExpressionSym;
class BoolSym;
class IntSym;
class NumSym;
class TimeSym;
class SizeSym;
class StringSym;
class StrRangeSym;
class ListSym;
class ContainerSym;
class ArraySym;
class NetAddrSym;
class NetAddrRangeSym;
class QualifSym;
class DistrSym;
class DynamicNameSym;
class MimeHeaderSym;
class PopDistrSym;
class RegExSym;

// interprets parsed PGL text
// kids interpret locale/mode-specific code such as use() function calls
class PglSemx {
	public:
		PglSemx();
		virtual ~PglSemx();

		virtual void interpret(const SynSym &s);
		static void WorkerId(int wokerId);
		static const char* WorkerIdStr();

	protected:
		void interpCode(const ParsSym &pgl);

		void interpStatementSeq(const ParsSym &pgl);
		void interpStatement(const ParsSym &pgl);

		void interpSimpleStatement(const ParsSym &pgl);

		void interpExprStatement(const ParsSym &pgl);
		void interpDeclStatement(const ParsSym &pgl);
		void interpPureDecl(const ParsSym &pgl);
		void interpCodeInitDecl(const ParsSym &pgl);
		void interpExprInitDecl(const ParsSym &pgl);

		void interpAssignment(const ParsSym &pgl);

		ExpressionSym *makeExpression(const ParsSym &pgl);
		ExpressionSym *makeBinExpr(const ExpressionSym &exp1, 
			const TokenSym &op, const ExpressionSym &exp2);

		ExpressionSym *makeQuotedConstant(const TokenSym &s);
		ExpressionSym *makeTypedConst(const ParsSym &pgl);
		BoolSym *makeBool(const TokenSym &s);
		IntSym *makeInt(const TokenSym &s);
		NumSym *makeNum(const TokenSym &s);
		TimeSym *makeTime(const TokenSym &s);
		SizeSym *makeSize(const TokenSym &s);
		ExpressionSym *makeString(const TokenSym &s);
		StringSym *makeStringAtom(const TokenSym &s);
		StrRangeSym *makeStringRange(const TokenSym &s);
		NetAddrSym *makeAddrAtom(const TokenSym &s);
		NetAddrRangeSym *makeAddrRange(const TokenSym &s);
		QualifSym *makeQualif(const TokenSym &s);
		MimeHeaderSym *makeMimeHeader(const TokenSym &s);
		RegExSym *makeRegEx(const TokenSym &scopeName, const TokenSym &reSym);

		ListSym *makeList(const ParsSym &pgl);
		ArraySym *makeArray(const ParsSym &pgl, bool ofNames = false);

		ExpressionSym *makeClone(const ExpressionSym &expr, const ExpressionSym &factor);

		void checkArgs(const String &cname, int expCount, const ListSym &args);
		const ExpressionSym &extractArg(const String &cname, int idx, const ListSym &args, const String &type);
		void argsToDouble(const String &cname, const ListSym &argsIn, Array<double> &outa);
		void argsToInt(const String &cname, const ListSym &argsIn, Array<int> &outa);

		ExpressionSym *makeFuncCall(const ParsSym &pgl);
		void interpProcCall(const ParsSym &pgl);
		void interpIfCode(const ParsSym &pgl);
		void interpEveryCode(const ParsSym &pgl);
		virtual void execEveryCode(const EveryCodeSym &ecode);
		virtual ExpressionSym *callFunc(const String &cname, const ListSym &args);
		virtual void callProc(const String &cname, const ListSym &args);
		void noCall(const String &cname, const ListSym &args);

		void openContext(const String &name);
		void closeContext();

		void addListItem(ListSym &list, const ParsSym &expr);
		void addArrayItem(ArraySym &arr, bool ofNames, const ParsSym &expr, double prob = -1.0);

		void assignToOne(const ParsSym &name, const ExpressionSym &expr);
		void assignToEach(const ParsSym &lhs, const ExpressionSym &expr);
		void assignCode(const ParsSym &name, const ParsSym &code);
		void assign(SynSymTblItem *i, const SynSym &newVal, const SrcLoc &loc);

		SynSymTblItem *findDescr(const String &name); // may fail
		SynSymTblItem *getDescr(const String &name, const TokenLoc &loc);
		SynSymTblItem *declare(const String &type, const String &name, const TokenLoc &loc);
		SynSym &getSym(const String &name, const TokenLoc &loc);

		void setDefault(SynSymTblItem *i);
		bool knownType(const String &type);

		ExpressionSym *calcExtreme(const String &cname, const ListSym &args, int dir);
		DistrSym *isDistr(const String &cname, const ListSym &args);
		PopDistrSym *isPopDistr(const String &cname, const ListSym &args);
		bool isToken(const SynSym &s, int tokenId) const;
		bool isToken(const SynSym &s) const;
		bool isRule(const SynSym &s, const char *name) const;
		bool isRule(const SynSym &s) const;

		ArraySym *calcAgentAddrs(const String &cname, const ListSym &args, AddrSchemeSym::AddrCalc calc);
		ArraySym *tracedHosts(const String &cname, const ListSym &args);
		ContainerSym *genCredentials(const String &cname, int count, const String &nameSpace, const TokenLoc &loc);
		ContainerSym *selectItems(const String &cname, const ContainerSym &items, int goalCount, const TokenLoc &loc);
		ContainerSym *setAddrPort(const String &cname, const ContainerSym &items, int newPort, const TokenLoc &loc);
		RegExSym *orItems(const ContainerSym &items);
		DynamicNameSym *dynamicName(const NetAddrSym &addr, const NumSym &prob);
		ContainerSym *dynamize(const String &cname, const ContainerSym &items, const NumSym &prob);
		ostream &print(ostream &os, const ListSym &args, const unsigned int skip = 0);

		String typeName(const ParsSym &pgl) const;
		String objName(const ParsSym &pgl) const;

		SynSym *anyToAny(const SynSym &s, const String &type) const;
		int anyToInt(const SynSym &s) const;
		double anyToDouble(const SynSym &s) const;
		void noCast(const SynSym &s, const String &totype) const;
		void failedCast(const SynSym &s, const String &totype, const String &str) const;

		void mustBeDefined(const ExpressionSym *const expr, const char *const descr, const SrcLoc &defaultLoc = SrcLoc()) const;
		void unknownRhs(const ParsSym &pgl) const;

	protected:
		PglCtx *theCtx;    // current context

		static int TheWorkerId;
		static String TheWorkerIdStr;
};

#endif
