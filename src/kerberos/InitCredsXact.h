
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__KERBEROS_INITCREDSXACT_H
#define POLYGRAPH__KERBEROS_INITCREDSXACT_H

#include "kerberos/Xact.h"

class UserCred;

namespace Kerberos {

class InitCredsXact: public Xact {
	public:
		InitCredsXact() { reset(); }

		virtual const ::String &description() const;
		virtual void reset();
		bool configure(Mgr &owner, const ::String &password);
		Time ticketEndTime() const { return theInitCredsContext.endTime(); }

		ErrorCode getCreds(Creds &creds);

	protected:
		virtual void finish(const bool isSuccessful = false);
		virtual ErrorCode step(Data &in, Data &out);
		virtual ::Error polyError() const;

		InitCredsContext theInitCredsContext;
};

}; // namespace Kerberos

#endif
