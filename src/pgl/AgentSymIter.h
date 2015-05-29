
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_AGENTITER_H
#define POLYGRAPH__PGL_AGENTITER_H

#include "xstd/Array.h"
#include "xstd/NetAddr.h"

class AgentSym;

// Iterates Agent configurations of the matching Agent type,
// optionally including matching right proxy-side configurations 
class AgentSymIter {
	public:
		typedef Array<AgentSym*> Agents;

	public:
		AgentSymIter(Agents &, const String &anAgentType, bool doIncludeProxies);
		~AgentSymIter();

		//bool eof() const;
		operator void *() const;
		AgentSymIter &operator ++(); // next agent

		const AgentSym *agent() const { return theAgents[thePos]; }
		int agentIdx() const { return thePos; }
		bool agentInProxy() const { return isAgentInProxy; }

	protected:
		void sync();

	private:
		Agents &theAgents; // all agents, matching or not
		String theAgentType; // matching agent type
		bool includeProxies; // whether to include matching proxy side

		bool isAgentInProxy; // whether agent() represents a proxy side
		int thePos;  // current iteration offset

};

#endif

