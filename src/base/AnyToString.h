
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_ANYTOSTRING_H
#define POLYGRAPH__BASE_ANYTOSTRING_H

#include "xstd/h/sstream.h"
#include "xstd/String.h"

// expensive but convenient convertion of any printable object to string
template <class T>
inline
String AnyToString(const T &any) {
	ostringstream os;
	os << any;
	return Stream2String(os);
}

// same for classes that have a print(ostream) method only
template <class T>
inline
String PrintToString(const T &any) {
	ostringstream os;
	any.print(os);
	return Stream2String(os);
}

// same for a container of pointers to printable things
template <class T>
inline
String PointersToString(const T &strings, const String &delimiter) {
	ostringstream os;
	for (int i = 0; i < strings.count(); ++i) {
		if (i)
			os << delimiter;
		os << *strings[i];
	}
	return Stream2String(os);
}

#endif
