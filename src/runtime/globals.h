
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_GLOBALS_H
#define POLYGRAPH__RUNTIME_GLOBALS_H

#include "xstd/LibInit.h"

// argv[0]
class String;
extern String ThePrgName;

class FileScanner;
extern FileScanner *TheFileScanner;

class UniqId;
extern UniqId TheGroupId;

LIB_INITIALIZER(GlobInit)

#endif
