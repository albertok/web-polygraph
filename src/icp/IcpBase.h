
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__ICP_ICPBASE_H
#define POLYGRAPH__ICP_ICPBASE_H

#include "xstd/NetAddr.h"
#include "xstd/Socket.h"
#include "xstd/FileScanner.h"

class IcpMsg;

// base class for ICP clients and servers
class IcpBase: FileScanUser {
	public:
		virtual ~IcpBase();

		void configure(const NetAddr &aHost);

		Socket &socket() { return theSock; }

		virtual void start();

		virtual void noteReadReady(int fd);

	protected:
		// both call noteUnexpMsg
		virtual void noteRequest(const IcpMsg &m);
		virtual void noteReply(const IcpMsg &m);

		void noteUnexpMsg(const IcpMsg &m);
		virtual int logCat() const = 0;

	protected:
		NetAddr theHost;
		Socket theSock;
		FileScanReserv theReserv;
};

#endif
