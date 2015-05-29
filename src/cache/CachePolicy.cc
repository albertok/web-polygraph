
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "cache/CachePolicy.h"


CachePolicy::CachePolicy(Cache *aCache): theCache(aCache) {
}

CachePolicy::~CachePolicy() {
}
