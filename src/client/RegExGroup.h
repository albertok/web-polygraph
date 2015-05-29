
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_REGEXGROUP_H
#define POLYGRAPH__CLIENT_REGEXGROUP_H

#include "xstd/h/iosfwd.h"

class String;
class RegExExpr;
class RegExMatchee;
class Memberships;

// an interface for a group of REs with possibly optimized matching algorithm
class RegExGroup {
	public:
		static RegExGroup *FromExpr(RegExExpr *expr);

	public:
		virtual ~RegExGroup() {}

		virtual bool match(const RegExMatchee &matchee, int flags = 0, int *errNo = 0) const = 0;
		virtual ostream &print(ostream &os) const = 0;
};

class RegExMatchee {
	public:
		const char *url;
		const char *urlHost;
		const char *urlPath;
		const char *userName;
		const Memberships *memberships;
};

#endif
