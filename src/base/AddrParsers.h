
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_ADDRPARSERS_H
#define POLYGRAPH__BASE_ADDRPARSERS_H

/* routines for parsing address-related, possibly non-terminated strings */

class NetAddr;

extern bool ParseNetAddr(const char *buf, const char *eoh, const int defaultPort, NetAddr &addr);
extern bool ParseDname(const char *buf, const char *eoh, NetAddr &addr);
extern bool ParseIPvFour(const char *buf, const char *eoh, NetAddr &addr);
extern bool ParseIPvSix(const char *buf, const char *eoh, NetAddr &addr);
extern const char *SkipHostInUri(const char *buf, const char *eorl, NetAddr &host);

#endif
