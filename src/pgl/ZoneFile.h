
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_ZONE_FILE_H
#define POLYGRAPH__PGL_ZONE_FILE_H

class ArraySym;
class String;
class ContainerSym;

extern ArraySym *ZoneDomains(const TokenLoc &loc, const String &fname, const ContainerSym &ips);

#endif
