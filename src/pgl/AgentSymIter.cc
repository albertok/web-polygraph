
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"
#include "pgl/AgentSymIter.h"

#include "pgl/AgentSym.h"
#include "pgl/ProxySym.h"


AgentSymIter::AgentSymIter(Agents &anAgents, const String &anAgentType, bool doIncludeProxies):
	theAgents(anAgents), theAgentType(anAgentType), 
	includeProxies(doIncludeProxies), isAgentInProxy(false),
	thePos(0) {
	sync();
}

AgentSymIter::~AgentSymIter() {
}

AgentSymIter::operator void *() const {
	return (thePos < theAgents.count()) ? (void*)(-1) : 0;
}

AgentSymIter &AgentSymIter::operator ++() {
	thePos++;
	sync();
	return *this;
}

// must be called after a cfg index change
void AgentSymIter::sync() {
	isAgentInProxy = false;
	for (; thePos < theAgents.count(); ++thePos) {
		const AgentSym *agent = theAgents[thePos];
		if (agent->isA(theAgentType))
			return;
	}
}
