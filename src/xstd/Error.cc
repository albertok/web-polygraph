
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/string.h"
#include "xstd/h/iostream.h"

#include "xstd/String.h"
#include "xstd/Array.h"
#include "xstd/Error.h"

static Array<String*> *TheCustomErrStrs = 0;

// Windows API guarantees that 29th bit in errno is unused
const int Error::TheCustomOff = (1 << 28);


static
Array<String*> &CustomErrStrs() {
	if (!TheCustomErrStrs)
		TheCustomErrStrs = new Array<String*>(1024);
	return *TheCustomErrStrs;
}


const Error Error::Last() {
#if defined(HAVE_GETLASTERROR)
	return Error(GetLastError());
#else
	return Error(errno);
#endif
}

const Error Error::Last(const Error &err) {
#if defined(HAVE_SETLASTERROR)
	SetLastError(err.no());
#else
	errno = err.no();
#endif
	return err;
}

const Error Error::LastExcept(const Error &exc) {
	const Error err = Last();
	return err == exc ? Error() : err;
}

const Error Error::Add(const char *str) {
	Assert(str);
	const int errNo = 1 + CustomErrStrs().count() + TheCustomOff;
	CustomErrStrs().append(new String(str));
	return Error(errNo);
}


bool Error::custom() const {
	return no() >= TheCustomOff;
}

const char *Error::str() const {
	if (custom()) {
		const int pos = no() - TheCustomOff - 1;
		if (0 <= pos && pos < CustomErrStrs().count())
			return CustomErrStrs()[pos]->cstr();
		else
			return "invalid custom errno";
	}

	/* system error */

#if defined(FORMAT_MESSAGE_FROM_SYSTEM)
	static char buf[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, no(), 0, buf, sizeof(buf), 0);
	return buf;
#else
	return strerror(no());
#endif
}


ostream &Error::print(ostream &os, const char *str) const {
	os << '(';
	if (custom())
		os << 'c' << (no() - TheCustomOff);
	else
		os << 's' << no();

	return os << ") " << str;
}
