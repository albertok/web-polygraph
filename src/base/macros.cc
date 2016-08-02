
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/String.h"

#include "base/macros.h"

// Simple macro expansion.
// Replaces all occurences of @macro@ in @str@ with @replacement@ and returns the
// resulting string.
// Note: @macro@ should begin with '$' or '%'.
String ExpandMacro(const String &str, const String &macro, const String &replacement) {
	const char startMacro = macro ? macro[0] : '\0';
	Assert(startMacro == '$' || startMacro == '%');
	const Area m(Area::Create(macro.cstr()));

	// a quick check before we start doing expensive preparations
	int searchStart = str.len() >= m.size() ?
		str.find(startMacro, 0) : String::npos;
	if (searchStart == String::npos) // no macros
		return str;

	String result;

	int headStart = 0; // we searched but did not process the head yet
	while (searchStart < str.len()) {
		const int pos = str.find(m, searchStart);
		if (pos == String::npos)
			break;
		result += str(headStart, pos); // head
		result += replacement;
		searchStart = headStart = pos + m.size();
	}

	if (headStart < str.len())
		result += str(headStart, str.len());  // tail

	return result;
}
