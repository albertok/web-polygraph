
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_GETIFADDRS_H
#define POLYGRAPH__XSTD_GETIFADDRS_H

#include "xstd/Array.h"
#include "xstd/Socket.h"

class InetIfReq;
class String;
class InAddress;

// retreives a list of all interface addresses and, optionally, netmasks
// using SIOCGIFCONF or an equivalent OS-specific system call
extern bool GetIfAddrs(Array<InetIfReq> &addrs, const String &ifname, Array<InAddress> *netmasks = 0);

#endif
