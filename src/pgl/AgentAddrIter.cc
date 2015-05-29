
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"
#include "pgl/AgentAddrIter.h"

#include "pgl/AgentSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrSym.h"

class AddrCollector: public ContainerSym::Visitor {
	public:
		AddrCollector(AgentAddrIter::Addresses &aStore);
		virtual void visit(const SynSym &item);
		AgentAddrIter::Addresses &theStore;
};


AgentAddrIter::AgentAddrIter(AgentSymIter::Agents &agents, const String &agentType):
	theSymIter(agents, agentType, true), theAddrPos(0) {
	sync();
}

AgentAddrIter::~AgentAddrIter() {
	resetAddresses();
}

bool AgentAddrIter::eof() const {
	return !theSymIter;
}

AgentAddrIter::operator void *() const {
	return !eof() ? (void*)(-1) : 0;
}

AgentAddrIter &AgentAddrIter::operator ++() {
	theAddrPos++;
	sync();
	return *this;
}

const NetAddr &AgentAddrIter::address() const {
	Assert(!eof());
	return theAddr;
}

const NetAddrSym *AgentAddrIter::addressSym() const {
	Assert(!eof());
	return theAddresses.item(theAddrPos);
}

// must be called after a host index change
void AgentAddrIter::sync() {
	for (; theSymIter; ++theSymIter) {
		if (theAddresses.empty()) {
			const AgentSym *agent = theSymIter.agent();
			if (const ArraySym *array = agent->addresses()) {
				AddrCollector v(theAddresses);
				array->forEach(v);
			}
			theAddrPos = 0;
		}
		if (theAddrPos < theAddresses.count()) {
			theAddr = theAddresses[theAddrPos]->val();
			return;
		}
		resetAddresses();
	}

	theAddr = NetAddr();
}

void AgentAddrIter::resetAddresses() {
	while (theAddresses.count()) delete theAddresses.pop();
}

AddrCollector::AddrCollector(AgentAddrIter::Addresses &aStore): theStore(aStore) {
}

void AddrCollector::visit(const SynSym &item) {
	const NetAddrSym *s = (const NetAddrSym*)item.clone(NetAddrSym::TheType);
	theStore.append(s);
}
