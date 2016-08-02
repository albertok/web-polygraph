
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/StatPhaseMgr.h"
#include "runtime/BcastSender.h"


void StopTrafficWaiting() {
    if (TheStatPhaseMgr.trafficWaiting())
		TheStatPhaseMgr.stopTrafficWaiting();
}
