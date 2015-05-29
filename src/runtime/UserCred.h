
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_USERCRED_H
#define POLYGRAPH__RUNTIME_USERCRED_H

#include "xstd/String.h"

class Connection;
class NetAddr;
class ObjId;

// maintains information about user credentials
class UserCred {
	public:
		UserCred(): isValid(true) {}
		UserCred(const String &anImage): theImage(anImage), isValid(true) {}

		void finalize(const ObjId &oid);
		void finalize(const Connection &conn);

		void reset() { theImage = String(); isValid = true; }

		bool valid() const { return isValid; }
		const String &image() const { return theImage; }
		Area name() const;
		Area password() const;

		void invalidate();

	protected:
		void finalize(const NetAddr &addr);

		String theImage;
		bool isValid;
};

#endif
