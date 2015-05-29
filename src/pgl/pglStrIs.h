
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSTRIS_H
#define POLYGRAPH__PGL_PGLSTRIS_H

class String;
class Time;
class BigSize;
class NetAddr;

extern bool pglIsRatio(const String &str, double &r);
extern bool pglIsRelTime(const String &str, Time &t);
extern bool pglIsAbsTime(const String &str, Time &t);
extern bool pglIsSize(const String &str, BigSize &s);
extern bool pglIsNetAddr(const String &str, NetAddr &a);

#endif
