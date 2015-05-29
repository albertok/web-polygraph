
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_NETPIPESYM_H
#define POLYGRAPH__PGL_NETPIPESYM_H

#include "pgl/HostsBasedSym.h"

// base class for robot and server symbols
class NetPipeSym: public HostsBasedSym {
	public:
		static String TheType;

	public:
		NetPipeSym();
		NetPipeSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		String kind() const;
		bool bandwidthBitsps(int &bps) const;
		bool queuePacketSize(int &size) const;
		bool queueByteSize(int &size) const;
		bool packetLoss(double &plr) const;
		RndDistr *packetDelay() const;
		bool incoming() const;
		bool outgoing() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
