
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/rndDistrs.h"
#include "xstd/StringIdentifier.h"
#include "xstd/h/iomanip.h"
#include "base/RndPermut.h"
#include "base/polyLogCats.h"
#include "runtime/HostMap.h"
#include "runtime/PopModel.h"
#include "runtime/ErrorMgr.h"
#include "runtime/LogComment.h"
#include "runtime/polyErrors.h"

#include "pgl/ProxySym.h"
#include "runtime/ProxyCfg.h"

ProxySharedCfgs TheProxySharedCfgs;

ProxyCfg::ProxyCfg(): theProxy(0) {
}

ProxyCfg::~ProxyCfg() {
}

void ProxyCfg::configure(const ProxySym *aProxy) {
	AgentCfg::configure(aProxy);

	Assert(!theProxy && aProxy);
	theProxy = aProxy;
}

bool ProxyCfg::sslActive() const {
	return !theSslWraps.empty();
}


/* ProxySharedCfgs */

ProxySharedCfgs::~ProxySharedCfgs() {
	while (count()) delete pop();
}

ProxyCfg *ProxySharedCfgs::getConfig(const ProxySym *rs) {
	for (int i = 0; i < count(); ++i) {
		if (item(i)->theProxy == rs)
			return item(i);
	}
	return addConfig(rs);
}

ProxyCfg *ProxySharedCfgs::addConfig(const ProxySym *rs) {
	ProxyCfg *cfg = new ProxyCfg;
	cfg->configure(rs);
	append(cfg);
	return cfg;
}
