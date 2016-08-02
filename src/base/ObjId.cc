
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/FNVHash.h"
#include "xstd/gadgets.h"
#include "xstd/Rnd.h"
#include "base/OLog.h"
#include "base/ILog.h"
#include "base/ObjId.h"
#include "base/RndPermut.h"


void ObjId::reset() {
	theForeignUrl = String();
	theWorld.clear();
	theName = theViserv = theType = -1;

	theTarget = -1;

	thePrefix = 0;

	theFlags = 0;
}

// XXX: change this and other hash() methods to return unsigned int
int ObjId::hash() const {
	const int defaultHash = 1000000711;

	if (theWorld && theName > 0) {
		// theType may not be known yet; moreover it is dirived from hash()
		const unsigned int hash = FNV32Hash(theName, theWorld.hash());
		return Abs(static_cast<int>(hash));
	} else
	if (Should(theForeignUrl)) {
		return theForeignUrl.hash();
	}

	return defaultHash;
}

bool ObjId::direct() const {
	return !embedded();
}

bool ObjId::embedded() const {
	Assert(theWorld);
	return theWorld.mutant();
}

// /*first*/ cachable miss
bool ObjId::fill() const {
	return basic() && /* !repeat() &&*/ cachable() && !hit();
}

ostream &ObjId::printFlags(ostream &os) const {
	if (repeat())
		os << "rpt,";
	if (hot())
		os << "hot,";
	if (foreignSrc())
		os << "foreignSrc,";
	if (foreignUrl())
		os << "foreignUrl,";

	if (basic())
		os << "basic,";
	if (ims200())
		os << "ims200,";
	if (ims304())
		os << "ims304,";
	if (reload())
		os << "reload,";
	if (rediredReq())
		os << "rediredReq,";
	if (repToRedir())
		os << "repToRedir,";
	if (aborted())
		os << "aborted,";

	if (get())
		os << "GET,";
	else
	if (head())
		os << "HEAD,";
	else
	if (post())
		os << "POST,";
	else
	if (put())
		os << "PUT,";

	if (cachable())
		os << "chb,";

	if (hit())
		os << "hit,";

	if (gzipContent())
		os << "gzip";

	return os;
}

void ObjId::store(OLog &ol) const {
	ol << theWorld << theViserv << theName << theType
		<< theTarget;
	ol.puti((int)theFlags);
}

void ObjId::load(ILog &il) {
	il >> theWorld >> theViserv >> theName >> theType
		>> theTarget;
	theFlags = (unsigned)il.geti();
}

bool ObjId::calcPublic(const double prob) {
	Assert(theName > 0);
	if (prob > 0) {
		static RndGen rng;
		rng.seed(GlbPermut(hash(), rndWorldSel));
		return rng.event(prob);
	}
	return false;
}

bool ObjId::parse(const char *&buf, const char *end) {
	// user-specified url_prefix may contain world-id tag;
	// search for the first tag that is followed by a valid wid
	while (const char *wid = StrBoundChr(buf, 'w', end))
		if (theWorld.parse(buf = wid+1, end))
			break;

	if (const char *tid = StrBoundChr(buf, 't', end))
		isInt(tid + 1, theType, &buf, 16);
	if (const char *oid = StrBoundChr(buf, '_', end))
		isInt64(oid + 1, theName, &buf, 16);

	// find the end of the Uri
	while (buf < end && !isspace(*buf))
		++buf;

	return theWorld && theType >= 0 && theName >= 0;
}
