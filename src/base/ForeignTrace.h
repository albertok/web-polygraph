
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_FOREIGNTRACE_H
#define POLYGRAPH__BASE_FOREIGNTRACE_H

#include "xstd/String.h"
#include "xstd/Array.h"

class NetAddr;

// manages a foreign access log or URL trace
// loads trace into RAM when converting to URLs or host names
class ForeignTrace {
	public:
		ForeignTrace();

		void configure(const String &aName, bool ignoreBad = true);

		int gatherUrls(Array<String*> &urls) const;
		int gatherHosts(Array<NetAddr*> &hosts) const;

	protected:
		String *getUrl(istream &is) const;
		bool open(istream &is) const;
		void close(const int goodCount) const;

	protected:
		String theName;
		bool doIgnoreBad;

		mutable int theEntryCount;
		mutable int theMemSize;
};

#endif
