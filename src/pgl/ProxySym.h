
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PROXYSYM_H
#define POLYGRAPH__PGL_PROXYSYM_H

#include "pgl/AgentSym.h"

class RobotSym;
class ServerSym;
class CacheSym;

// server side configuration
class ProxySym: public AgentSym {
	public:
		static const String TheType;

	public:
		ProxySym();
		ProxySym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		RobotSym *client() const;
		ServerSym *server() const;
		AgentSym *side(const String &sideType) const;
		CacheSym *cache() const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
		virtual String msgTypesField() const;
};

#endif
