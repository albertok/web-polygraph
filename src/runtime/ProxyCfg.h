
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2013 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_PROXYCFG_H
#define POLYGRAPH__RUNTIME_PROXYCFG_H

#include "xstd/Array.h"
#include "runtime/AgentCfg.h"

class ProxySym;

// Proxy configuration items that can be shared among multiple proxies.
// Used to configure Robot side of SSL-to-proxy connections.
class ProxyCfg: public AgentCfg {
	public:
		ProxyCfg();
		~ProxyCfg();

		void configure(const ProxySym *aProxy);

		// whether SSL is configured and supported for protocol
		bool sslActive() const;

	public:
		const ProxySym *theProxy;  // used to identify/share configs
};


class ProxySharedCfgs: protected Array<ProxyCfg*> {
	public:
		~ProxySharedCfgs();
		ProxyCfg *getConfig(const ProxySym *cfg);

	protected:
		ProxyCfg *addConfig(const ProxySym *cfg);
};

extern ProxySharedCfgs TheProxySharedCfgs;

#endif
