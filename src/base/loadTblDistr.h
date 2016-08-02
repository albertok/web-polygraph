
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_LOADTBLDISTR_H
#define POLYGRAPH__BASE_LOADTBLDISTR_H

class String;
class RndDistr;

// XXX: move to parser module?
// If a non-empty distribution type is given, it must match the loaded type.
// Upon successful return, distributionType will be set to the loaded type.
extern RndDistr *LoadTblDistr(const String &fname, String &distributionType);

#endif
