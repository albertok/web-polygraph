
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_HOSTSBASEDITER_H
#define POLYGRAPH__PGL_HOSTSBASEDITER_H

#include "xstd/NetAddr.h"
#include "pgl/AgentSymIter.h"

class AgentSym;
class NetAddrSym;

// Iterates addresses of Agents of the right agent type
// The class builds optimized iteration table for each agent because large
// number of agent addresses make brute-force iteration expensive.
class AgentAddrIter {
	public:
		typedef Array<const NetAddrSym*> Addresses;

	public:
		AgentAddrIter(AgentSymIter::Agents &, const String &anAgentType);
		~AgentAddrIter();

		bool eof() const;
		operator void *() const;
		AgentAddrIter &operator ++(); // next address

		const NetAddr &address() const;   // current address
		const NetAddrSym *addressSym() const;
		const AgentSym *agent() const { return theSymIter.agent(); }
		int agentIdx() const  { return theSymIter.agentIdx(); }

	protected:
		void sync();
		void resetAddresses();

	private:
		AgentSymIter theSymIter;

		Addresses theAddresses; // current agent's addresses
		NetAddr theAddr; // cached current address value
		int theAddrPos; // address offset within the current agent
};

#endif
