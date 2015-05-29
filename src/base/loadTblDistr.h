
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_LOADTBLDISTR_H
#define POLYGRAPH__BASE_LOADTBLDISTR_H

class String;
class RndDistr;

// XXX: move to parser module?
extern RndDistr *LoadTblDistr(const String &fname, const String &argType);

#endif
