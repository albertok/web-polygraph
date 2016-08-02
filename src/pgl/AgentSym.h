
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_AGENTSYM_H
#define POLYGRAPH__PGL_AGENTSYM_H

#include "base/UniqId.h"
#include "pgl/PglNumSym.h"
#include "pgl/HostsBasedSym.h"

class MimeHeaderSym;
class StringSym;
class SocketSym;
class PopModelSym;

// base class for robot and server symbols
class AgentSym: public HostsBasedSym {
	public:
		static String TheType;

	public:
		AgentSym(const String &aType);
		AgentSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		String kind() const;
		UniqId world() const;
		RndDistr *httpVersions(const TokenIdentifier &versionNames) const;
		SocketSym *socket() const;
		RndDistr *xactLifetime() const;
		RndDistr *xactThink() const;
		RndDistr *pconnUseLmt() const;
		Time idlePconnTimeout() const;
		PopModelSym *popModel() const;
		bool msgTypes(Array<StringSym*> &types, Array<double> &tprobs) const;
		RndDistr *msgTypes(const TokenIdentifier &typeNames) const;
		bool abortProb(double &p) const;
		bool cookieSender(double &prob) const;
		bool customStatsScope(Array<StringSym*> &syms) const;
		const ArraySym *httpHeaders() const;

	protected:
		virtual String msgTypesField() const = 0;
};

#endif
