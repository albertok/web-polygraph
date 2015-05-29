
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_IPSTONAMES_H
#define POLYGRAPH__PGL_IPSTONAMES_H

#include "xstd/Array.h"
#include "xparser/LexToken.h"

class ContainerSym;
class ArraySym;

extern ArraySym *IpsToNames(const ContainerSym &ips, const String &dname);

#endif
