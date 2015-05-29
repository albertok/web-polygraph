
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/string.h"
#include "xstd/h/iostream.h"
#include "xstd/h/string.h"

#include "xstd/Assert.h"
#include "xstd/Area.h"


Area Area::Create(const char *str) {
	return str ? Area::Create(str, 0, strlen(str)) : Area("", 0, 0);
}

Area &Area::operator =(const Area &) {
	Assert(false); // disabled
	return *this;
}


ostream &operator <<(ostream &os, const Area &area) {
	if (area)
		os.write(area.data(), area.size());
	return os;
}
