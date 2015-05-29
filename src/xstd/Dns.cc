
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/String.h"
#include "xstd/Dns.h"

hostent *Dns::GetHostByName(const String &name) {
	return gethostbyname(name.cstr());
}
