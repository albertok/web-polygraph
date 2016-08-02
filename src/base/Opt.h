
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_OPT_H
#define POLYGRAPH__BASE_OPT_H

#include "xstd/Array.h"
#include "xstd/String.h"

class Opt;
class CmdLine;


// a group of command line options
class OptGrp: public Array<Opt*> {
	public:
		virtual ~OptGrp() {}

		virtual ostream &printAnonym(ostream &os) const { return os; }
		virtual bool parseAnonym(const Array<const char *> &) { return true; }
		virtual bool canParseAnonym() const { return false; }

		virtual bool validate() const { return true; }
		virtual String ExpandMacros(const Opt &opt, const String &str) const { return str; }

		void share(OptGrp *other);
};

// a generic option
class Opt {
	public:
		Opt(OptGrp *aGrp, const char *aName, const char *aDescr);
		virtual ~Opt();

		void cmdLine(CmdLine *aCmdLine) { theCmdLine = aCmdLine; }

		bool set(const String &name, const String &val);
		virtual void report(ostream &) const = 0;
		virtual bool visible() const { return true; } // use in reports

		const String &name() const { return theName; }
		const String &type() const { return theType; }
		const String &descr() const { return theDescr; }
		bool wasSet() const { return isSet; }

	protected:
		virtual bool parse(const String &name, const String &val) = 0;

		OptGrp *theGrp;
		CmdLine *theCmdLine;
		String theName;
		String theType;
		String theDescr;
		bool isSet; // explicitly configured rather than relying on default
};


#endif
