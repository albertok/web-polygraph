
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/math.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/String.h"
#include "pgl/PglRec.h"
#include "pgl/PglStringSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/BenchSym.h"
#include "pgl/PolyMix3AsSym.h"



const String PolyMix3AsSym::TheType = "PolyMix3As";

static const String strPolyMix3 = "PolyMix-3";


PolyMix3AsSym::PolyMix3AsSym(): VerThreeAsSym(TheType, new PglRec) {
	kind(strPolyMix3);
}

PolyMix3AsSym::PolyMix3AsSym(const String &aType, PglRec *aRec): VerThreeAsSym(aType, aRec) {
}

bool PolyMix3AsSym::isA(const String &type) const {
	return VerThreeAsSym::isA(type) || type == TheType;
}

SynSym *PolyMix3AsSym::dupe(const String &type) const {
	if (isA(type))
		return new PolyMix3AsSym(this->type(), theRec->clone());
	return VerThreeAsSym::dupe(type);
}

String PolyMix3AsSym::robots(ArraySym *&addrs) const {
	return addresses(theBench->clientSide(), 0, addrs);
}

String PolyMix3AsSym::servers(ArraySym *&addrs) const {
	return addresses(theBench->serverSide(), 128, addrs);
}
