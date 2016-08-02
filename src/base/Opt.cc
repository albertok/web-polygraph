
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Assert.h"
#include "base/Opt.h"


/* OptGrp */

void OptGrp::share(OptGrp *other) {
	Assert(other);
	for (int i = 0; i < count(); ++i)
		other->append(item(i));
}


/* Opt */

Opt::Opt(OptGrp *aGrp, const char *aName, const char *aDescr):
	theGrp(aGrp), theCmdLine(0), theName(aName), theDescr(aDescr),
	isSet(false) {

	if (const char *h = theName.chr(' ')) {
		theType = theName(h+1 - theName.cstr(), theName.len());
		theName = theName(0, h - theName.cstr());
	} else {
		theType = String();
	}

	if (theGrp)
		theGrp->append(this); // self registration
}

Opt::~Opt() {
}

bool Opt::set(const String &name, const String &val) {
	isSet = true;
	return parse(name, theGrp->ExpandMacros(*this, val));
}
