
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <ctype.h>
#include "xstd/h/string.h"

#include "xstd/h/regex.h"
#include "xstd/h/iostream.h"

#include "xstd/Assert.h"
#include "xstd/RegEx.h"


RegEx::RegEx(): theFlags(0), theError(0), isCompiled(false) {
}

RegEx::~RegEx() {
	if (isCompiled && (theFlags & kdReMatch))
		regfree(this);
}

void RegEx::configure(const String &image, const String &pattern, int flags) {
	Assert(!isCompiled);
	theImage = image;
	theFlags = flags;
	thePattern = pattern;
	theError = 0;

	if (flags & reExact)
		confStrCmp();
	else
	if (!hasReChars(pattern))
		confStrStr();
	else
		confReMatch();

	isCompiled = !theError;
}

void RegEx::confStrCmp() {
	theFlags |= kdStrCmp;
}

void RegEx::confStrStr() {
	theFlags |= kdStrStr;
}

void RegEx::confReMatch() {
	theFlags |= kdReMatch;

	int internalFlags = 0;
#	if defined(REG_EXTENDED)
		// we will add a custom flag later if we need to disable this default
		internalFlags |= REG_EXTENDED;
#	endif
#	if defined(REG_ICASE)
		if (theFlags & reIgnoreCase)
			internalFlags |= REG_ICASE;
#	endif

	String internalPattern = thePattern;
	if (theFlags & reExact) {
		// note: we should not get here because of StrCmp optimization
#		if defined(REG_NOSPEC)
			internalFlags |= REG_NOSPEC; // note: REG_NOSPEC is not standard,
			internalFlags &= ~REG_EXTENDED; // incomatible with REG_EXTENDED
#		else
			// escape all special characters
			for (int i = 0; i < thePattern.len(); ++i) {
				if (!isalnum(thePattern[i]))
					internalPattern += '\\';
				internalPattern += thePattern[i];
			}
#		endif
	}

	theError = regcomp(this, internalPattern.cstr(), internalFlags);
	//Error::Last(theError);
	if (theError) {
		char buf[16*1024];
		regerror(theError, this, buf, sizeof(buf));
		print(cerr << here << "regex '") << "' error: " << buf << endl;
	}
}

bool RegEx::valid() const {
	return isCompiled && !theError;
}

bool RegEx::configured() const {
	return theImage.len() > 0 || thePattern.len() > 0;
}

bool RegEx::match(const char *str, int flags, int *errNo) const {
	bool res = false;

	if (Should(!theError)) {
		if (errNo)
			*errNo = 0;

		if (theFlags & kdStrCmp) {
			res = strCmp(str);
		} else
		if (theFlags & kdStrStr) {
			res = strStr(str);
		} else {
			Matches ms;
			res = reMatch(str, ms, flags);
		}
	}
	if (errNo)
		*errNo = theError;

	return res;
}

bool RegEx::match(const char *str, Matches &matches, int flags, int *errNo) const {
	const bool res = Should(!theError) && reMatch(str, matches, flags);
	if (errNo)
		*errNo = theError;
	return res;
}

bool RegEx::strCmp(const char *str) const {
	if (theFlags & reIgnoreCase)
		return strncasecmp(str, thePattern.data(), thePattern.len()) == 0;
	else
		return strncmp(str, thePattern.data(), thePattern.len()) == 0;
}

bool RegEx::strStr(const char *str) const {
	if (theFlags & reIgnoreCase)
		return strcaseStr(str) != 0;
	else
		return strstr(str, thePattern.cstr()) != 0;
}

bool RegEx::reMatch(const char *str, Matches &matches, int flags) const {
	int iFlags = 0;
	Match correction;
	correction.rm_so = 0;
	
	if ((flags & reStartEnd) != 0 && Should(matches.count() > 0)) {
#		if defined(REG_STARTEND)
			iFlags |= REG_STARTEND;
#		else
			// REG_STARTEND RE extension is not supported
			// we need to terminate the buffer and re-adjust offsets after
			static String buf;
			buf = String();
			buf.append(str + matches[0].rm_so,
				matches[0].rm_eo - matches[0].rm_so);
			str = buf.cstr();
			correction.rm_so = matches[0].rm_so;
#		endif
	}
	matches.resize(matches.capacity());
	const int res =
		regexec(this, str, matches.count(), matches.items(), iFlags);
	if (res == 0) {
		// readjust offsets if we were faking REG_STARTEND support above
		if (correction.rm_so > 0) {
			for (int i = 0; i < matches.count(); ++i) {
				matches[i].rm_so += correction.rm_so;
				matches[i].rm_eo += correction.rm_so;
			}
		}
		return true;
	}
#if defined(REG_NOMATCH)
	if (res == REG_NOMATCH)
		return false;
#endif
	theError = res;
	if (theError) {
		char buf[16*1024];
		regerror(theError, this, buf, sizeof(buf));
		print(cerr << here << "regex '") << "' error: " << buf << endl;
	}
	return false;
}

bool RegEx::hasReChars(const String &pattern) const {
	for (int i = 0; i < pattern.len(); ++i) {
		const char c = pattern[i];

		if (c == '\\') {
			++i;
			continue;
	}
		
		if (c == '|' || c == '*' || c == '+' || c == '?')
			return true;
		if (c == '{' || c == '[' || c == '(')
			return true;
	}

	return false;
}

const char *RegEx::strcaseStr(const char *str) const {
	if (const char first = thePattern[0]) {
		const char *rest = thePattern.data() + 1;
		const char len = thePattern.len() - 1;
		do {
			// find first character of a pattern in str
			char c;
			do {
				c = *str++;
				if (!c)
					return 0;
			} while (c != first);
		} while (strncasecmp(str, rest, len) != 0);
		--str;
	}
	return str;
}

RegEx::Match RegEx::StartEnd(Size offset, Size size) {
	Match res;
	res.rm_so = offset;
	res.rm_eo = offset + size;
	return res;
}
