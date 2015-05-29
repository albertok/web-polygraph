
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLRECSYM_H
#define POLYGRAPH__PGL_PGLRECSYM_H

#include "xstd/Time.h"
#include "xstd/NetAddr.h"
#include "xstd/Array.h"
#include "pgl/PglBwidthSym.h"
#include "pgl/PglExprSym.h"

class BigSize;
class String;
class RndDistr;
class StringArray;
class TokenIdentifier;
class PglRec;
class ArraySym;
class ContainerSym;

class RecSym: public ExpressionSym {
	public:
		typedef PglRec Rec;

	public:
		static String TheType;

	public:
		RecSym(const String &aType, Rec *aRec): ExpressionSym(aType), theRec(aRec) {}
		virtual ~RecSym();

		Rec *rec() { return theRec; }
		virtual SynSymTblItem **memberItem(const String &name);

		virtual bool isA(const String &type) const;

		// no fields set
		bool empty() const;
		// true if array contains elem with non-zero probability
		bool arrayHasElem(const String &array, const String &elem) const;

		virtual ostream &print(ostream &os, const String &pfx) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;

		bool getBool(const String &name, bool &set) const;
		bool getInt(const String &name, int &n) const;
		bool getDouble(const String &name, double &n) const;
		bool getSize(const String &name, BigSize &sz) const;
		bool getRate(const String &name, double &rate) const;
		bool getBwidth(const String &name, Bwidth &bw) const;
		Time getTime(const String &name) const;
		RndDistr *getDistr(const String &name) const;
		String getString(const String &name) const;
		NetAddr getNetAddr(const String &name) const;
		bool getNetAddrs(const String &name, Array<NetAddr*> &) const;
		bool getStrings(const String &name, Array<String*> &strs, Array<double> *const probs = 0) const;
		bool getStringArray(const String &name, StringArray &strs) const;
		RndDistr *namesToDistr(const String &fieldName, const TokenIdentifier &names) const;

		ContainerSym *getContainerSym(const String &name) const;
		ArraySym *getArraySym(const String &name) const;
		RecSym *getRecSym(const String &name, bool emptyOk = false) const;

	protected:
		Rec *theRec;
};

#endif
