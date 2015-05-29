
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__ICP_ICPPROXY_H
#define POLYGRAPH__ICP_ICPPROXY_H

#include "icp/IcpClient.h"
#include "icp/IcpServer.h"

// combines ICP client and server capabilities
class IcpProxy: public IcpClient, public IcpServer {
	protected:
		virtual int logCat() const { return IcpClient::logCat(); }

		virtual void noteRequest(const IcpMsg &m) { IcpServer::noteRequest(m); }
		virtual void noteReply(const IcpMsg &m) { IcpClient::noteReply(m); }
};

#endif
