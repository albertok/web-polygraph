
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "dns/DnsMsg.h"

DnsMsg::Field DnsMsg::TheLastId = 0;


DnsMsg::Field DnsMsg::NextId() {
	if (++TheLastId == 0)
		TheLastId = 1;
	return TheLastId;
}

DnsMsg::DnsMsg(): theId(0) {
}

void DnsMsg::reset() {
	theQueryAddr = NetAddr();
	theId = 0;
}
