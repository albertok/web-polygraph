
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_STRINGIDENTIFIER_H
#define POLYGRAPH__XSTD_STRINGIDENTIFIER_H

#include <map>
#include "xstd/TokenIdentifier.h"

// StringIdentifier is a simple String unique integer ID mapp.
// For efficient prefix-only lookup see PrefixIdentifier.
class StringIdentifier: public TokenIdentifier {
public:

		virtual int lookup(const char *buf, int len) const;
		virtual int lookup(const String &str) const;

	protected:
		virtual void doAdd(String &str, int id);

		typedef std::map<String, int> Map; // maps String: id
		Map theMap;
};

#endif
