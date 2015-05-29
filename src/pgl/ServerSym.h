
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_SERVERSYM_H
#define POLYGRAPH__PGL_SERVERSYM_H

#include "pgl/AgentSym.h"

class StringSym;
class ContentSym;
class RndDistr;

// server side configuration
class ServerSym: public AgentSym {
	public:
		static const String TheType;

	public:
		ServerSym();
		ServerSym(const String aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool acceptLmt(int &lmt) const;
		bool contents(Array<ContentSym*> &ccfgs, Array<double> &cprobs) const;
		bool directAccess(Array<ContentSym*> &dacfgs, Array<double> &daprobs) const;

		bool cookieSetProb(double &prob) const;
		RndDistr *cookieSetCount() const;
		RndDistr *cookieValueSize() const;
		bool reqBodyAllowed(double &f) const;
		RndDistr *protocols(const TokenIdentifier &protocolNames) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
		virtual String msgTypesField() const;
};

#endif
