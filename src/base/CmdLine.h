
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_CMDLINE_H
#define POLYGRAPH__BASE_CMDLINE_H

#include "xstd/String.h"
#include "xstd/Array.h"

class Opt;
class OptGrp;

// command line parser
class CmdLine {
	public:
		CmdLine();
		virtual ~CmdLine();

		void configure(const Array<OptGrp*> &groups);
		bool parse(int argc, char *argv[]);

		void usage(ostream &os) const;
		void report(ostream &os) const; // both raw and parsed
		void reportRaw(ostream &os) const;
		void reportParsed(ostream &os) const;

	protected:
		// parse one option at a time
		bool parse(const String &name, const String &val);

	protected:
		Array<String *> theArgs;
		Array<Opt *> theOpts;
		OptGrp *theAnonymParser; // group to use for anonymous options

		String thePrgName;   // program name (argv[0])
};

#endif
