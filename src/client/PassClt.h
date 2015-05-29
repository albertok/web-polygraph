
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_PASSCLT_H
#define POLYGRAPH__CLIENT_PASSCLT_H

#include "client/Client.h"

class PassClt: public Client {
	public:
		PassClt();
		virtual void wakeUp(const Alarm &);
		virtual void scheduleLaunch(Time lastLaunch);
		virtual bool launchCanceled(CltXact *x);
};

#endif
