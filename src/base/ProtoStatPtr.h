
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_PROTOSTATPTR_H
#define POLYGRAPH__BASE_PROTOSTATPTR_H

// avoid including StatIntvlRec.h in all headers that use ProtoIntvlPtr
class ProtoIntvlStat;
class StatIntvlRec;
typedef ProtoIntvlStat StatIntvlRec::*ProtoIntvlPtr;

#endif
