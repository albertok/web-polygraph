
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_POLYVERSION_H
#define POLYGRAPH__BASE_POLYVERSION_H

class String;

// stuff to report package version, compilation time, and such.

extern const String &PolyVersion();
extern const String &PolyCompDate();
extern const String &PolyCompTime();
extern const String &PolyLetterhd();

#endif
