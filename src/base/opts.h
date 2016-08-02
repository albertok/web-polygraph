
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_OPTS_H
#define POLYGRAPH__BASE_OPTS_H

#include "xstd/String.h"
#include "xstd/BigSize.h"
#include "xstd/NetAddr.h"
#include "xstd/Rnd.h"
#include "base/Progress.h"
#include "base/Opt.h"

// bool
class BoolOpt: public Opt {
	public:
		BoolOpt(OptGrp *aGrp, const char *aName, const char *aDescr, bool def = false);

		virtual void report(ostream &os) const;

		operator bool() const { return theVal; }

	protected:
		virtual bool parse(const String &name, const String &val);

		bool theVal;
};

// int
class IntOpt: public Opt {
	public:
		IntOpt(OptGrp *aGrp, const char *aName, const char *aDescr, int def = -1);

		virtual void report(ostream &os) const;

		operator int() const { return theVal; }

		// not an assignment op to prevent accidents
		void set(int aVal) { theVal = aVal; }

	protected:
		virtual bool parse(const String &name, const String &val);

		int theVal;
};

// double
class DblOpt: public Opt {
	public:
		DblOpt(OptGrp *aGrp, const char *aName, const char *aDescr, double def = -1);

		virtual void report(ostream &os) const;

		operator double() const { return theVal; }

	protected:
		virtual bool parse(const String &name, const String &val);

		double theVal;
};

// Time
class TimeOpt: public Opt, public Time {
	public:
		TimeOpt(OptGrp *aGrp, const char *aName, const char *aDescr, Time def = Time());

		virtual void report(ostream &os) const;

		void set(Time aVal) { theVal = aVal; }

	protected:
		virtual bool parse(const String &name, const String &val);

		Time &theVal;
};

// Size
class SizeOpt: public Opt, public Size {
	public:
		SizeOpt(OptGrp *aGrp, const char *aName, const char *aDescr, Size def = Size());

		virtual void report(ostream &os) const;

		// not an assignment op to prevent accidents
		void set(Size aVal) { theVal = aVal; }

	protected:
		virtual bool parse(const String &name, const String &val);

		Size &theVal;
};

// BigSize
class BigSizeOpt: public Opt, public BigSize {
	public:
		BigSizeOpt(OptGrp *aGrp, const char *aName, const char *aDescr, BigSize def = BigSize());

		virtual void report(ostream &os) const;

		// not an assignment op to prevent accidents
		void set(const BigSize &aVal) { theVal = aVal; }

	protected:
		virtual bool parse(const String &name, const String &val);

		BigSize &theVal;
};

// String
class StrOpt: public Opt, public String {
	public:
		StrOpt(OptGrp *aGrp, const char *aName, const char *aDescr, const char *def = 0);

		virtual void report(ostream &os) const;

		void val(const String &aVal);

	protected:
		virtual bool parse(const String &name, const String &val);

		String &theVal;
};

// NetAddr
class NetAddrOpt: public Opt, public NetAddr {
	public:
		NetAddrOpt(OptGrp *aGrp, const char *aName, const char *aDescr, const NetAddr &def);

		virtual void report(ostream &os) const;

	protected:
		virtual bool parse(const String &name, const String &val);

		NetAddr &theVal;
};

// Rate
class RateOpt: public Opt {
	public:
		RateOpt(OptGrp *aGrp, const char *aName, const char *aDescr);

		virtual void report(ostream &os) const;

		operator void *() const { return set() ? (void*)-1 : 0; }
		bool set() const { return theInterval > 0; }
		double rate() const; // [1/second]
		Time intArrTime() const; // inter-arrival time

	protected:
		virtual bool parse(const String &name, const String &val);

		double theCount;
		Time theInterval;
};

// Distr
class DistrOpt: public Opt {
	public:
		DistrOpt(OptGrp *aGrp, const char *aName, const char *aDescr, RndDistr *def = 0);

		operator void *() const { return theVal ? (void*)-1 : 0; }
		RndDistr *distr() { Assert(theVal); return theVal; }
		RndDistr *condDistr() { return theVal; }

		virtual void report(ostream &os) const;

		double trial() { Assert(theVal); return theVal->trial(); }
		double operator ()() { return trial(); }

	protected:
		virtual bool parse(const String &name, const String &val);

		RndDistr *theVal;
};


// Help
class HelpOpt: public Opt {
	public:
		HelpOpt(OptGrp *aGrp, const char *aName, const char *aDescr);

		virtual void report(ostream &) const {}
		virtual bool visible() const { return false; }

	protected:
		virtual bool parse(const String &name, const String &val);
};

// Two Int
class TwoIntOpt: public Opt {
	public:
		TwoIntOpt(OptGrp *aGrp, const char *aName, const char *aDescr);

		virtual void report(ostream &os) const;

		bool set() const { return lo() <= hi(); }
		int hi() const { return theHiVal; }
		int lo() const { return theLoVal; }

	protected:
		virtual bool parse(const String &name, const String &val);

		int theLoVal;
		int theHiVal;
};


// basic List
class ListOpt: public Opt {
	public:
		ListOpt(OptGrp *aGrp, const char *aName, const char *aDescr, char aDel = ',');

		virtual bool addItem(const String &item) = 0;

	protected:
		virtual bool parse(const String &name, const String &val);

		char theDel;
};


// String array
class StrArrOpt: public ListOpt {
	public:
		StrArrOpt(OptGrp *aGrp, const char *aName, const char *aDescr);

		operator void*() const { return theVal.count() ? (void*)-1 : 0; }
		const Array<String*> &val() const { return theVal; }
		void copy(Array<String*> &val) const;

		virtual void report(ostream &os) const;
		virtual bool addItem(const String &item);

	protected:
		PtrArray<String*> theVal;
};

#endif
