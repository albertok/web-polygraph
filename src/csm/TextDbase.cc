
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "csm/TextDbase.h"


TextDbase::TextDbase() {
}

TextDbase::~TextDbase() {
	while (theStrs.count())
		delete theStrs.pop();
}

void TextDbase::name(const String &aName) {
	theName = aName;
}

void TextDbase::load(istream &is) {
	char buf[16*1024];
	while (is.getline(buf, sizeof(buf))) {
		if (const int len = strlen(buf))
			add(buf, len);
	}
}

void TextDbase::add(const char *str, int len) {
	char *dup = new char[len+1];
	strncpy(dup, str, len);
	theStrs.append(dup);
	theLens.append(len);
}
