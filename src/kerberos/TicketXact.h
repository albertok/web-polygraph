
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__KERBEROS_TICKETXACT_H
#define POLYGRAPH__KERBEROS_TICKETXACT_H

#include "kerberos/Xact.h"

class CltXact;

namespace Kerberos {

class TicketXact: public Xact {
	public:
		TicketXact() { reset(); }

		virtual const ::String &description() const;
		virtual void reset();
		bool configure(Mgr &owner, CCache &ccache, const NetAddr &serviceAddr, CltXact &aReason);
		CltXact &reason() { Must(theReason); return *theReason; }
		const ::String &serviceName() const { return theServiceName; }

	protected:
		virtual void finish(const bool isSuccessful = false);
		virtual ErrorCode step(Data &in, Data &out);
		virtual ::Error polyError() const;

		CltXact *theReason;
		::String theServiceName;
		TktCredsContext theTktCredsContext;
};

}; // namespace Kerberos

#endif
