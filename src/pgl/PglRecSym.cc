
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/TblDistr.h"
#include "xstd/TokenIdentifier.h"
#include "base/StringArray.h"
#include "base/StringArrayBlocks.h"
#include "pgl/PglCtx.h"
#include "pgl/PglRec.h"
#include "pgl/PglRecSym.h"
#include "pgl/PglBoolSym.h"
#include "pgl/PglIntSym.h"
#include "pgl/PglNumSym.h"
#include "pgl/PglSizeSym.h"
#include "pgl/PglRateSym.h"
#include "pgl/PglBwidthSym.h"
#include "pgl/PglDistrSym.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglStringArrayBlockSym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglTimeSym.h"
#include "pgl/PglArraySym.h"

class StringArrayMaker: public ContainerSym::Visitor {
	public:
		StringArrayMaker(StringArray &aStore);
		virtual void visit(const SynSym &item);
		StringArray &theStore;
};

String RecSym::TheType = "Rec";

static String strDistr = "Distr";


RecSym::~RecSym() {
	delete theRec;
}

bool RecSym::isA(const String &type) const {
	return ExpressionSym::isA(type) || type == TheType;
}

SynSym *RecSym::dupe(const String &type) const {
	Assert(theRec);

	if (isA(type))
		return new RecSym(this->type(), theRec->clone());
	else
		return ExpressionSym::dupe(type);
}

bool RecSym::empty() const {
	const int symCount = theRec->count();
	for (int i = 0; i < symCount; ++i) {
		if (const SynSym *ss = theRec->valAt(i)->sym()) {
			// empty() is recursive for items that are records
			if (ss->isA(TheType) && ((const RecSym&)ss->cast(TheType)).empty())
				continue;
			return false;
		}
	}
	return true;
}

// Slow! Should only be used for small arrays such as type selectors
bool RecSym::arrayHasElem(const String &array, const String &elem) const {
	Array<const StringSym*> types;
	Array<double> tprobs;

	if (ArraySym *a = getArraySym(array)) {
		a->copyProbs(tprobs);
		a->exportA(types);
		for (int i = 0; i < types.count(); ++i)
			if (types[i]->val() == elem &&
				tprobs[i] > 0)
				return true;
	}

	return false;
}

SynSymTblItem **RecSym::memberItem(const String &name) {
	return rec()->valp(name);
}

ostream &RecSym::print(ostream &os, const String &pfx) const {
	os << '{' << endl;
	if (theRec)
		theRec->report(os, pfx + "    ");
	os << pfx << '}';
	return os;
}

bool RecSym::getBool(const String &name, bool &set) const {
	SynSymTblItem *bi = 0;
	Assert(theRec);
	Assert(theRec->find(name, bi));
	if (bi->sym())
		set = ((const BoolSym&)bi->sym()->cast(BoolSym::TheType)).val();
	return bi->sym() != 0;
}

bool RecSym::getInt(const String &name, int &n) const {
	SynSymTblItem *ii = 0;
	Assert(theRec);
	Assert(theRec->find(name, ii));
	if (ii->sym())
		n = ((const IntSym&)ii->sym()->cast(IntSym::TheType)).val();
	return ii->sym() != 0;
}

bool RecSym::getDouble(const String &name, double &n) const {
	SynSymTblItem *di = 0;
	Assert(theRec);
	Assert(theRec->find(name, di));
	if (di->sym())
		n = ((const NumSym&)di->sym()->cast(NumSym::TheType)).val();
	return di->sym() != 0;
}

Time RecSym::getTime(const String &name) const {
	SynSymTblItem *ti = 0;
	Assert(theRec);
	Assert(theRec->find(name, ti));
	return ti->sym() ?
		((const TimeSym&)ti->sym()->cast(TimeSym::TheType)).val() : Time();
}

RndDistr *RecSym::getDistr(const String &name) const {
	const String typeName = strDistr;
	SynSymTblItem *di = 0;
	Assert(theRec);
	Assert(theRec->find(name, di));
	return di->sym() ?
		((const DistrSym&)di->sym()->cast(typeName)).val() : 0;
}

String RecSym::getString(const String &name) const {
	SynSymTblItem *ni = 0;
	Assert(theRec->find(name, ni));
	return ni->sym() ?
		((const StringSym&)ni->sym()->cast(StringSym::TheType)).val() : String();
}

NetAddr RecSym::getNetAddr(const String &name) const {
	SynSymTblItem *ni = 0;
	Assert(theRec->find(name, ni));
	return ni->sym() ?
		((const NetAddrSym&)ni->sym()->cast(NetAddrSym::TheType)).val() : NetAddr();
}

bool RecSym::getSize(const String &name, BigSize &sz) const {
	SynSymTblItem *ni = 0;
	Assert(theRec->find(name, ni));
	if (!ni->sym())
		return false;
	sz = ((const SizeSym&)ni->sym()->cast(SizeSym::TheType)).val();
	return true;
}

bool RecSym::getRate(const String &name, double &rate) const {
	SynSymTblItem *ri = 0;
	Assert(theRec->find(name, ri));
	if (ri->sym())
		rate = ((const RateSym&)ri->sym()->cast(RateSym::TheType)).val();
	return ri->sym() != 0;
}

bool RecSym::getBwidth(const String &name, Bwidth &bw) const {
	SynSymTblItem *bi = 0;
	Assert(theRec->find(name, bi));
	if (!bi->sym())
		return false;
	bw = ((const BwidthSym&)bi->sym()->cast(BwidthSym::TheType)).val();
	return true;
}

bool RecSym::getNetAddrs(const String &name, Array<NetAddr*> &addrs) const {
	SynSymTblItem *si = 0;
	Assert(theRec->find(name, si));
	if (!si->sym())
		return false;

	ArraySym &ss = (ArraySym&)si->sym()->cast(ArraySym::TheType);
	for (int i = 0; i < ss.count(); ++i) {
		const NetAddr &addr = ((const NetAddrSym&)ss[i]->cast(NetAddrSym::TheType)).val();
		addrs.append(new NetAddr(addr));
	}

	return true;
}

ContainerSym *RecSym::getContainerSym(const String &name) const {
	SynSymTblItem *contsi = 0;
	Assert(theRec);
	Assert(theRec->find(name, contsi));
	if (!contsi->sym())
		return 0;

	ContainerSym &cont = (ContainerSym&)contsi->sym()->cast(ContainerSym::TheType);
	return cont.count() > 0 ? &cont : 0;
}

ArraySym *RecSym::getArraySym(const String &name) const {
	SynSymTblItem *arrsi = 0;
	Assert(theRec);
	theRec->find(name, arrsi);
	if (!arrsi || !arrsi->sym())
		return 0;

	ArraySym &arr = (ArraySym&)arrsi->sym()->cast(ArraySym::TheType);
	return arr.count() > 0 ? &arr : 0;
}

bool RecSym::getStrings(const String &name, Array<String*> &strs, Array<double> *const probs) const {
	if (ArraySym *as = getArraySym(name)) {
		const int count = as->count();
		strs.stretch(strs.count() + count);
		for (int i = 0; i < count; ++i) {
			const String &str = ((const StringSym&)as->item(i)->cast(StringSym::TheType)).val();
			strs.append(new String(str));
		}
		if (probs)
			as->copyProbs(*probs);
		return true;
	}
	return false;
}

bool RecSym::getStringArray(const String &name, StringArray &strs) const {
	if (ContainerSym *s = getContainerSym(name)) {
		StringArrayMaker m(strs);
		s->forEach(m);
		return true;
	}
	return false;
}

RndDistr *RecSym::namesToDistr(const String &fieldName, const TokenIdentifier &names) const {
	Assert(names.count());

	SynSymTblItem *fti = 0;
	Assert(theRec->find(fieldName, fti));
	if (!fti->sym())
		return 0; // undefined

	// build selector using label index
	ArraySym &nts = (ArraySym&)fti->sym()->cast(ArraySym::TheType);
	Array<double> probs_in(names.count());
	Array<double> probs_out(names.count());
	nts.copyProbs(probs_in);

	for (int p = 0; p < probs_in.count(); ++p) {
		const String &name = ((const StringSym&)nts[p]->cast(StringSym::TheType)).val();

		const int id = names.lookup(name);
		if (id <= 0) {
			cerr << loc() << "unknown quoted name `" << name
				<< "'; known names within " << fieldName << " context are:";
			for (TokenIdentifier::Iter i = names.iterator(); i; ++i)
				cerr << " \"" << i.str() << '"';
			cerr << endl;
			exit(-2);
		}

		if (probs_in[p] > 0)
			probs_out.put(probs_in[p], id);
	}

	if (!probs_out.count())
		return 0;

	return TblDistr::FromDistrTable(type() + "-" + fieldName, probs_out);
}

RecSym *RecSym::getRecSym(const String &name, bool emptyOk) const {
	SynSymTblItem *rsi = 0;
	Assert(theRec);
	Assert(theRec->find(name, rsi));
	if (!rsi->sym())
		return 0;

	RecSym &rs = (RecSym&)rsi->sym()->cast(TheType);
	return rs.empty() && !emptyOk ? 0 : &rs;
}

StringArrayMaker::StringArrayMaker(StringArray &aStore): theStore(aStore) {
}

void StringArrayMaker::visit(const SynSym &item) {
	if (item.isA(StringArrayBlockSym::TheType)) { 
		const StringArrayBlockSym &a = (const StringArrayBlockSym&)item.cast(StringArrayBlockSym::TheType);
		theStore.absorb(a.val().clone());
	} else {
		const StringSym &str = (const StringSym&)item.cast(StringSym::TheType);
		theStore.append(str.val());
	}
}
