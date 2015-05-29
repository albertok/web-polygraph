
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_HTTPFINDCRLF_H
#define POLYGRAPH__RUNTIME_HTTPFINDCRLF_H

// returns true and sets beg/end offsets if CR*LF is found
extern bool HttpFindCrlf(const char *buf, Size size, Size &crlfBeg, Size &crlfEnd);

#endif
