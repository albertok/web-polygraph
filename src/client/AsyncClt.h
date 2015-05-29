
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_ASYNCCLT_H
#define POLYGRAPH__CLIENT_ASYNCCLT_H

#include "client/Client.h"

class AsyncClt: public Client {
	public:
		AsyncClt(RndDistr *aThinkDistr);
		virtual void configure(const RobotSym *cfg, const NetAddr &aHost);
		virtual void wakeUp(const Alarm &);

		virtual void becomeIdle();

		virtual void scheduleLaunch(Time lastLaunch);
		virtual bool launchCanceled(CltXact *x);
};

#endif
