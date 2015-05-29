
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/HttpFindCrlf.h"

bool HttpFindCrlf(const char *buf, Size size, Size &crlfBeg, Size &crlfEnd) {
	Size crOff = -1;
	for (Size i = 0; i < size; ++i) {
		const char c = buf[i];
		if (crOff < 0) { // looking for the first CR or LF
			if (c == '\n') {
				crlfBeg = i;
				crlfEnd = i + Size(1);
				return true;
			}

			if (c == '\r')
				crOff = i;
		} else { // skipping CRs, looking for the first LF
			if (c == '\n') {
				crlfBeg = crOff;
				crlfEnd = i + Size(1);
				return true;
			}

			if (c != '\r')
				crOff = -1;
		}
	}
	return false;
}
