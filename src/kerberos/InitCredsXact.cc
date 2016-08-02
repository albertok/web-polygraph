
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "kerberos/InitCredsXact.h"
#include "kerberos/Mgr.h"
#include "runtime/UserCred.h"
#include "runtime/polyErrors.h"

namespace Kerberos {

const ::String &InitCredsXact::description() const {
	static const ::String s = "acquiring initial credentials";
	return s;
}

void InitCredsXact::reset() {
	theInitCredsContext.free();
	Xact::reset();
}

bool InitCredsXact::configure(Mgr &owner, const ::String &password) {
	if (const ErrorCode e = theInitCredsContext.init(owner.principal())) {
		reportError(errKerberosCredsInit, "creating context", e);
		return false;
	}

	if (const ErrorCode e =
		theInitCredsContext.password(::String(password))) {
		reportError(errKerberosCredsInit, "setting password", e);
		return false;
	}

	return Xact::configure(owner);
}

ErrorCode InitCredsXact::getCreds(Creds &creds) {
	return creds.init(theInitCredsContext);
}

void InitCredsXact::finish(const bool isSuccessful) {
	Xact::finish(isSuccessful);
	owner().noteXactDone(*this, isSuccessful);
}

ErrorCode InitCredsXact::step(Data &in, Data &out) {
	const ErrorCode e = theInitCredsContext.step(in, out);
	if (e && (e != KRB5KRB_ERR_RESPONSE_TOO_BIG || usingTcp))
		reportError(errKerberosCredsInit, "assembling request", e);
	return e;
}

::Error InitCredsXact::polyError() const {
	return errKerberosCredsInit;
}

}; // namespace Kerberos
