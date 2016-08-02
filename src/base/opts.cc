
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Assert.h"
#include "xstd/rndDistrs.h"
#include "xstd/gadgets.h"
#include "base/UniqId.h"
#include "base/CmdLine.h"
#include "base/loadTblDistr.h"
#include "base/opts.h"


/* opts tools */

static
const char *valStr(const String &val) {
	return val ? val.cstr() : "[none]";
}

static
bool splitVal(const String &val, String &pref, String &suf, const char del = ':', const bool forward = true) {
	if (val) {
		if (const char *const h = forward ? val.chr(del) : val.rchr(del)) {
			pref = val(0, h - val.cstr());
			suf = val(h+1 - val.cstr(), val.len());
			return true;
		} else
			pref = val;
	}
	return false;
}

static
bool isRatio(const String &str, double &r) {
	if (str) {
		double h;
		const char *ptr = 0;
		if (isNum(str.cstr(), h, &ptr)) {
			if (!*ptr) { // nothing at the end
				r = h;
				return true;
			} else
			if (!strcmp(ptr, "%") || !strcmp(ptr, "p")) { // percent
				r = h / 100;
				return true;
			}
		}
	}
	return false;
}

static
bool isTime(const char *str, Time &t) {
	// note: things are set up so that "sec" stands for "1sec" 
	//       (convenient for rates)
	const char *ptr = str;
	int i = 1;
	double d;

	// try to preserve integer precision if possible
	const bool isi = !isDbl(str, d, &ptr);

	if (isi)
		isInt(str, i, &ptr);

	if (!*ptr && isi && i == 0)
		t = Time(0,0);
	else
	if (!strcmp(ptr, "msec") || !strcmp(ptr, "ms"))
		t = isi ? Time::Msec(i) : Time::Secd(d/1e3);
	else
	if (!strcmp(ptr, "sec") || !strcmp(ptr, "s"))
		t = isi ? Time::Sec(i) : Time::Secd(d);
	else
	if (!strcmp(ptr, "min"))
		t = isi ? Time::Sec(i*60) : Time::Mind(d);
	else
	if (!strcmp(ptr, "hour") || !strcmp(ptr, "hr"))
		t = isi ? Time::Sec(i*60*60) : Time::Hourd(d);
	else
	if (!strcmp(ptr, "day"))
		t = isi ? Time::Sec(i*24*60*60) : Time::Dayd(d);
	else
	if (!strcmp(ptr, "year"))
		t = isi ? Time::Sec(i*365*24*60*60) : Time::Yeard(d);
	else
		return false;

	return true;
}

static
bool isSize(const char *str, Size &s) {
	// note: things are set up so that "KB" stands for "1KB" 
	//       (convenient for rates)
	const char *ptr = str;
	int i = 1;

	const bool isi = isInt(str, i, &ptr);

	if (isi && !*ptr && i == 0)
		s = 0;
	else
	if (!strcmp(ptr, "byte") || !strcmp(ptr, "bytes") || !strcmp(ptr, "B"))
		s = i;
	else
	if (!strcmp(ptr, "KB") || !strcmp(ptr, "K"))
		s = Size::KB(i);
	else
	if (!strcmp(ptr, "MB"))
		s = Size::MB(i);
	else
		return false;

	return true;
}

static
bool isBigSize(const char *str, BigSize &s) {
	// note: things are set up so that "KB" stands for "1KB" 
	//       (convenient for rates)
	const char *ptr = str;
	int i = 1;

	const bool isi = isInt(str, i, &ptr);

	if (isi && !*ptr && i == 0)
		s = 0;
	else
	if (!strcmp(ptr, "Bytes") || !strcmp(ptr, "B"))
		s = BigSize(i);
	else
	if (!strcmp(ptr, "KB"))
		s = BigSize(1024*i);
	else
	if (!strcmp(ptr, "MB"))
		s = BigSize::MB(i);
	else
	if (!strcmp(ptr, "GB"))
		s = BigSize::MB(1024*i);
	else
		return false;

	return true;
}

// converts various types to double if possible
// XXX: val->double->val conversion/casts are bad, rewrite
static
bool ToDouble(const String &param, double &val) {
	const char *p = 0;
	if (isNum(param.cstr(), val, &p) && p && !*p)
		return true;

	Time t;
	if (isTime(param.cstr(), t)) {
		val = t.secd();
		return true;
	}

	Size s;
	if (isSize(param.cstr(), s)) {
		val = s;
		return true;
	}

	return false; // unknown param type
}

// format: type(p1,p2,...) or type:p1,p2,...
static
bool isDistr(const char *name, const char *val, RndDistr *&distr) {
// convenience kludge
	if (val) {

		distr = 0;

		if (!strcmp(val, "none"))
			return true;

		// now must set custom distribution or fail

		String v = strchr(val, ')') ? String(val)(0,strlen(val)-1) : (String)val;
		String type, params;

		if (splitVal(v, type, params, ':') || splitVal(v, type, params, '(')) {
			String s[3];
			double p[3];

			RndGen *gen = new RndGen;

			if (type == "unif") {
				if (splitVal(params, s[0], s[1], ',') &&
					ToDouble(s[0], p[0]) && ToDouble(s[1], p[1]))
					distr = new UnifDistr(gen, p[0], p[1]);
			} else
			if (type == "exp") {
				if (ToDouble(params, p[0]))
					distr = new ExpDistr(gen, p[0]);
			} else
			if (type == "norm") {
				if (splitVal(params, s[0], s[1], ',') &&
					ToDouble(s[0], p[0]) && ToDouble(s[1], p[1]))
					distr = new NormDistr(gen, p[0], p[1]);
			} else
			if (type == "logn") {
				if (splitVal(params, s[0], s[1], ',') &&
					ToDouble(s[0], p[0]) && ToDouble(s[1], p[1]))
					distr = LognDistr::ViaMean(gen, p[0], p[1]);
			} else
			if (type == "const") {
				if (ToDouble(params, p[0]))
					distr = new ConstDistr(gen, p[0]);
			} else
			if (type == "zipf") {
				int h;
				if (isInt(params.cstr(), h) && h > 0)
					distr = new ZipfDistr(gen, h);
			} else
			if (type == "seq") {
				int h;
				if (isInt(params.cstr(), h) && h > 0)
					distr = new SeqDistr(gen, h);
			} else
			if (type == "table") {
				// Deal with single- and two-parameter table() calls.
				// See PglSemx::isDistr() for similar code.
				String fname;
				String distrType;
				if (splitVal(params, s[0], s[1], ',')) { // table(fname,dtype)
					fname = s[0];
					distrType = s[1] + "_distr";
				} else { // table(fname)
					fname = params;
				}

				distr = LoadTblDistr(fname, distrType);
				delete gen; gen = 0;
				if (!distr) {
					cerr << "failed to load table distribution from " << valStr(fname) << "'" << endl;
					return false;
				}
			} else {
				delete gen;
				cerr << "unknown distribution `" << type << "' for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
				return false;
			}

			if (distr)
				return true;

			delete gen;

			cerr << "bad parameters for `" << type << "' distribution in the `" << name << "` option; got `" << valStr(params) << "'" << endl;
			// fall through to print examples
		}
	}
	cerr << "distribution value (e.g., `exp(13KB)' or `norm:3sec,1sec' or `table:/tmp/my.dist') expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;
}



/* Bool */

BoolOpt::BoolOpt(OptGrp *aGrp, const char *aName, const char *aDescr, bool def):
	Opt(aGrp, aName, aDescr), theVal(def) {
}

bool BoolOpt::parse(const String &name, const String &val) {
	if (val) {
		if (val == "1" || val == "yes" || val == "y" || val == "on") {
			theVal = true;
			return true;
		} else
		if (val == "0" || val == "no" || val == "n" || val == "off") {
			theVal = false;
			return true;
		}
	}
	cerr << "boolean (1|0|yes|no|on|off) value expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;
}

void BoolOpt::report(ostream &os) const {
	os << (theName[0] == 'd' ? (theVal ? "yes" : "no") : (theVal ? "on" : "off"));
}

/* Int */

IntOpt::IntOpt(OptGrp *aGrp, const char *aName, const char *aDescr, int def):
	Opt(aGrp, aName, aDescr), theVal(def) {
}

bool IntOpt::parse(const String &name, const String &val) {
	if (val) {
		if (val == "none") {
			theVal = -1;
			return true;
		} else
		if (val == "max") {
			theVal = 0x7fffffff;
			return true;
		} else {
			int h;
			const char *ptr = 0;
			if (isInt(val.cstr(), h, &ptr) && !*ptr) { // no garbage at the end
				theVal = h;
				return true;
			}
		}
	}

	cerr << "integer value expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;
}

void IntOpt::report(ostream &os) const {
	os << theVal;
}


/* Double */

DblOpt::DblOpt(OptGrp *aGrp, const char *aName, const char *aDescr, double def):
	Opt(aGrp, aName, aDescr), theVal(def) {
}

bool DblOpt::parse(const String &name, const String &val) {
	if (val) {
		if (val == "none") {
			theVal = -1;
			return true;
		} else
		if (isRatio(val, theVal))
			return true;
	}
	
	cerr << "double/ratio value (e.g., `0.23', `23%', or `23p') expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;
}

void DblOpt::report(ostream &os) const {
	os << theVal;
}


/* Time */

TimeOpt::TimeOpt(OptGrp *aGrp, const char *aName, const char *aDescr, Time def):
	Opt(aGrp, aName, aDescr), Time(def), theVal(*this) {
}

bool TimeOpt::parse(const String &name, const String &val) {
	if (val) {
		if (val == "none") {
			theVal = Time();
			return true;
		} else
		if (val == "inf") {
			theVal = Time(0x7fffffff, 0);
			return true;
		} else
		if (isTime(val.cstr(), theVal))
			return true;
	}
	cerr << "time value (e.g., '30msec') expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;

}

void TimeOpt::report(ostream &os) const {
	os << theVal;
}


/* Size */

SizeOpt::SizeOpt(OptGrp *aGrp, const char *aName, const char *aDescr, Size def):
	Opt(aGrp, aName, aDescr), Size(def), theVal(*this) {
}

bool SizeOpt::parse(const String &name, const String &val) {
	if (val) {
		if (val == "none") {
			theVal = Size();
			return true;
		} else
		if (val == "inf") {
			theVal = 0x7fffffff;
			return true;
		} else
		if (isSize(val.cstr(), theVal))
			return true;
	}
	cerr << "size value (e.g., '10KB') expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;

}

void SizeOpt::report(ostream &os) const {
	os << theVal;
}


/* BigSize */

BigSizeOpt::BigSizeOpt(OptGrp *aGrp, const char *aName, const char *aDescr, BigSize def):
	Opt(aGrp, aName, aDescr), BigSize(def), theVal(*this) {
}

bool BigSizeOpt::parse(const String &name, const String &val) {
	if (val) {
		if (val == "none") {
			theVal = BigSize();
			return true;
		} else
		if (isBigSize(val.cstr(), theVal))
			return true;
	}
	cerr << "big size value (e.g., `100MB' or '10GB') expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;
}

void BigSizeOpt::report(ostream &os) const {
	os << theVal;
}


/* String */

StrOpt::StrOpt(OptGrp *aGrp, const char *aName, const char *aDescr, const char *def):
	Opt(aGrp, aName, aDescr), String(def), theVal(*this) {
}

bool StrOpt::parse(const String &name, const String &val) {
	if (val) {
		if (val == "unique")
			theVal = UniqId::Create().str();
		else
			theVal = val;
		return true;
	}
	cerr << "string value expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;
}

void StrOpt::val(const String &aVal) {
	theVal = aVal;
}

void StrOpt::report(ostream &os) const {
	os << valStr(theVal);
}


/* NetAddr */

NetAddrOpt::NetAddrOpt(OptGrp *aGrp, const char *aName, const char *aDescr, const NetAddr &def):
	Opt(aGrp, aName, aDescr), NetAddr(def), theVal(*this) {
}

bool NetAddrOpt::parse(const String &name, const String &val) {
	String pref, suf;
	if (val && splitVal(val, pref, suf, ':', false)) {
		int p;
		if (isInt(suf.cstr(), p) && p >= 0) {
			theVal.addr(pref);
			theVal.port(p);
			return true;
		} else
			cerr << "invalid port specification for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	} else
		cerr << "net address (<host>:<port>) value expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;
}

void NetAddrOpt::report(ostream &os) const {
	os << theVal;
}


/* Rate */

RateOpt::RateOpt(OptGrp *aGrp, const char *aName, const char *aDescr):
	Opt(aGrp, aName, aDescr), theCount(0) {
}

bool RateOpt::parse(const String &name, const String &val) {
	String pref, suf;
	if (val && splitVal(val, pref, suf, '/')) {
		if (isNum(pref.cstr(), theCount) && theCount >= 0 && isTime(suf.cstr(), theInterval))
			return true;
	}
	cerr << "rate value (e.g. `5/sec') expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;
}

double RateOpt::rate() const {
	return theInterval > 0 ? Ratio(theCount, theInterval.secd()) : -1;
}

Time RateOpt::intArrTime() const {
	return theCount > 0 ? theInterval/theCount : Time();
}

void RateOpt::report(ostream &os) const {
	os << rate() << "/sec";
}


/* Distr */

DistrOpt::DistrOpt(OptGrp *aGrp, const char *aName, const char *aDescr, RndDistr *def):
	Opt(aGrp, aName, aDescr), theVal(def) {
}

// format: type(p1,p2,...) or type:p1,p2,...
bool DistrOpt::parse(const String &name, const String &val) {
	return isDistr(name.cstr(), val.cstr(), theVal);
}


void DistrOpt::report(ostream &os) const {
	if (theVal)
		os << *theVal;
	else
		os << "<none>";
}


/* HelpOpt */

HelpOpt::HelpOpt(OptGrp *aGrp, const char *aName, const char *aDescr):
	Opt(aGrp, aName, aDescr) {
}

bool HelpOpt::parse(const String &, const String &) {
	Assert(theCmdLine);
	theCmdLine->usage(cout);
	exit(0);
	return false;
}


/* TwoInt */

TwoIntOpt::TwoIntOpt(OptGrp *aGrp, const char *aName, const char *aDescr):
	Opt(aGrp, aName, aDescr), theLoVal(-1), theHiVal(-2) {
}

bool TwoIntOpt::parse(const String &name, const String &val) {
	if (val) {
		String pref, suf;
		if (val == "none") {
			theLoVal = -1;
			theHiVal = -2;
			return true;
		} else
		if (splitVal(val, pref, suf)) {
			if (isInt(pref.cstr(), theLoVal) && isInt(suf.cstr(), theHiVal)) {
				if (theLoVal >= 0 && theHiVal >= 0 && theLoVal <= theHiVal)
					return true;
			}
		} 
	}
	cerr << "<low>:<hi> expected for the `" << name << "` option; got `" << valStr(val) << "'" << endl;
	return false;
}

void TwoIntOpt::report(ostream &os) const {
	if (set())
		os << theLoVal << ':' << theHiVal;
	else
		os << "<none>";
}


/* List */

ListOpt::ListOpt(OptGrp *aGrp, const char *aName, const char *aDescr, char aDel):
	Opt(aGrp, aName, aDescr), theDel(aDel) {
}

bool ListOpt::parse(const String &name, const String &val) {
	if (val) {
		String v = val;
		for (String tail, token; splitVal(v, token, tail, theDel); v = tail) {
			if (!addItem(token))
				return false;
		}
		return !v || addItem(v); // leftovers
	}
	cerr << "`" << theDel << "'-separated list expected for the `"
		<< name << "' option; got nothing" << endl;
	return false;
}


/* String  array */

StrArrOpt::StrArrOpt(OptGrp *aGrp, const char *aName, const char *aDescr):
	ListOpt(aGrp, aName, aDescr, ',') {
}

bool StrArrOpt::addItem(const String &item) {
	theVal.append(new String(item));
	return true;
}

void StrArrOpt::copy(Array<String*> &cp) const {
	for (int i = 0; i < theVal.count(); ++i)
		cp.append(theVal[i]);
}

void StrArrOpt::report(ostream &os) const {
	for (int i = 0; i < theVal.count(); ++i) {
		if (i) os << ',';
		os << *theVal[i];
	}
}
