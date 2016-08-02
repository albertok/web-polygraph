
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "kerberos/Mgr.h"
#include "kerberos/TicketXact.h"
#include "runtime/polyErrors.h"

namespace Kerberos {

const ::String &TicketXact::description() const {
	static const ::String s = "getting ticket";
	return s;
}

void TicketXact::reset() {
	theReason = 0;
	theServiceName = ::String();
	theTktCredsContext.free();
	Xact::reset();
}

bool TicketXact::configure(Mgr &owner, CCache &ccache, const NetAddr &serviceAddr, CltXact &aReason) {
	theReason = &aReason;
	theServiceName = "HTTP/" + serviceAddr.addrA() + '@' + owner.realm();

	Principal servicePrincipal;
	if (!owner.parsePrincipal(serviceName(), servicePrincipal))
		return false;

	if (const ErrorCode e = theTktCredsContext.init(owner.principal(),
		servicePrincipal, ccache)) {
		reportError(errKerberosTicket, "creating context", e);
		return false;
	}

	return Xact::configure(owner);
}

void TicketXact::finish(const bool isSuccessful) {
	Xact::finish(isSuccessful);
	owner().noteXactDone(*this, isSuccessful);
}

ErrorCode TicketXact::step(Data &in, Data &out) {
	const ErrorCode e = theTktCredsContext.step(in, out);
	if (e && (e != KRB5KRB_ERR_RESPONSE_TOO_BIG || usingTcp))
		reportError(errKerberosTicket, "assembling request", e);
	return e;
}

::Error TicketXact::polyError() const {
	return errKerberosTicket;
}

}; // namespace Kerberos
