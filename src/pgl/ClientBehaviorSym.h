
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_CLIENTBEHAVIORSYM_H
#define POLYGRAPH__PGL_CLIENTBEHAVIORSYM_H

#include "pgl/RobotSym.h"

// client side configuration
class ClientBehaviorSym: public RobotSym {
	public:
		static const String TheType;

	public:
		ClientBehaviorSym();
		ClientBehaviorSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
