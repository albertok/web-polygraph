
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_SYNCCLT_H
#define POLYGRAPH__CLIENT_SYNCCLT_H

#include "client/Client.h"

class SyncClt: public Client {
	public:
		virtual void wakeUp(const Alarm &a);
		virtual void scheduleLaunch(Time lastLaunch);
		virtual bool launchCanceled(CltXact *x);

	protected:
		virtual void loneXactFollowup();
};

#endif
