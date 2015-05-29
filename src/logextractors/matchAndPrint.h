
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGEXTRACTORS_MATCHANDPRINT_H
#define POLYGRAPH__LOGEXTRACTORS_MATCHANDPRINT_H

#include "xstd/h/iosfwd.h"
#include "xstd/Array.h"

class String;

// scans buf for matching objects and prints them
// prints the entire buffer if no names were specified
// retuns number of matches or -1 if no names were specified
extern int MatchAndPrint(ostream &os, const char *buf, const Array<String*> &names);

// same as MatchAndPrint but extracts and prints object bodies only
extern int MatchAndPrintBodies(ostream &os, const char *buf, const Array<String*> &names);

// same as MatchAndPrint but extracts and prints object tags only
extern int MatchAndPrintHeader(ostream &os, const char *buf, const Array<String*> &names);

#endif
