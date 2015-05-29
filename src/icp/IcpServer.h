
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__ICP_ICPSERVER_H
#define POLYGRAPH__ICP_ICPSERVER_H

#include "icp/IcpBase.h"

class Cache;

// replies to ICP queries
class IcpServer: virtual public IcpBase {
	public:
		IcpServer();

		void cache(Cache *aCache);

	protected:
		void noteRequest(const IcpMsg &m);
		virtual int logCat() const;

	protected:
		Cache *theCache;
};

#endif
