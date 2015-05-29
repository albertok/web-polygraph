
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__SERVER_POLYSRV_H
#define POLYGRAPH__SERVER_POLYSRV_H

#include "app/PolyApp.h"

class PolySrv: public PolyApp {
	public:
		PolySrv();

	protected:
		virtual void configure();
		
		virtual void getOpts(Array<OptGrp*> &opts);
		virtual Agent *makeAgent(const AgentSym &agent, const NetAddr &address);
		virtual void logState(OLog &log);

		virtual const String sideName() const;
		virtual int logCat() const;
};

#endif
