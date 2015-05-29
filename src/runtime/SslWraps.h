
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_SSLWRAPS_H
#define POLYGRAPH__RUNTIME_SSLWRAPS_H

#include "runtime/ConfigSymMgr.h"
#include "runtime/SslWrap.h"
#include "pgl/SslWrapSym.h"

// prevents creation of large number of identical SslWrap descriptions
class SslWraps: public ConfigSymMgr<SslWrapSym, SslWrap> {
	protected:
		virtual SslWrap *makeCfg();
};

extern SslWraps TheSslWraps;

#endif
