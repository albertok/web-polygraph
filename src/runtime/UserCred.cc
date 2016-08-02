/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "base/macros.h"
#include "base/ObjId.h"
#include "base/AddrParsers.h"
#include "runtime/Connection.h"
#include "runtime/HostMap.h"
#include "runtime/UserCred.h"
#include "pgl/PglSemx.h"

// Not in csm/oid2Url.h because pgl2ldif cannot link with that w/o bringing
// in too many dependencies with it
// XXX: move to oidAlgos along with half of the other oid2Url.h functions
static
NetAddr Oid2Target(const ObjId &oid) {
	if (oid.foreignUrl()) {
		const char *furi = oid.foreignUrl().data();
		NetAddr host;
		if (Should(SkipHostInUri(furi, furi+oid.foreignUrl().len(), host)))
			return host;
	} else
	if (const HostCfg *const host = TheHostMap->at(oid.target())) {
		return host->theAddr;
	}

	Should(false);
	return NetAddr();
}


const String UserCred::TheTargetIpMacro = "${target_ip}";

void UserCred::finalize(const ObjId &oid) {
	finalize(Oid2Target(oid));
}

void UserCred::finalize(const Connection &conn) {
	finalize(conn.raddr());
}

Area UserCred::name(bool &hasRealm) const {
	hasRealm = false;

	if (!isValid)
		return Area::Empty();

	int pos;
	for (pos = 0; pos < theImage.len(); ++pos) {
		if (theImage[pos] == '@')
			hasRealm = true;
		else
		if (theImage[pos] == ':')
			break;
	}
	return theImage.area(0, pos);
}

Area UserCred::password() const {
	if (!isValid)
		return Area::Empty(); // TODO: be more creative

	if (const char *p = theImage.chr(':')) {
		if (++p < theImage.data() + theImage.len())
			return theImage.area(p - theImage.data(), theImage.len());
	}
	return Area::Empty(); // no ":" or nothing after ":"
}

void UserCred::invalidate() {
	if (theImage.len() > 0)
		theImage += theImage.last();
	isValid = false;
}

// does macro substitutions
void UserCred::finalize(const NetAddr &addr) {
	const String &replacement = addr.addrA();
	Should(replacement.len());
	theImage = ExpandMacro(theImage, TheTargetIpMacro, replacement);
	theImage = ExpandMacro(theImage, "${worker}", PglSemx::WorkerIdStr());
}
