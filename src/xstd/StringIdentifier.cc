
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/String.h"
#include "xstd/StringIdentifier.h"


int StringIdentifier::lookup(const char *buf, int len) const {
	return lookup(String(buf, len));
}

int StringIdentifier::lookup(const String &str) const {
	const Map::const_iterator i = theMap.find(str);
	return i == theMap.end() ? 0 : i->second;
}

void StringIdentifier::doAdd(String &str, int id) {
	theMap.insert(std::make_pair(str, id));
}
