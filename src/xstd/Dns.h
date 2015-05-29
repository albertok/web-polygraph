
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_DNS_H
#define POLYGRAPH__XSTD_DNS_H

#include "xstd/h/netdb.h"

class String;

// gethostbyname() and friends wrapper
class Dns {
	public:
		/*static const Error Last();
		 *static const Error Last(const Error &err);
		 */

		static hostent *GetHostByName(const String &name);

};

#endif
