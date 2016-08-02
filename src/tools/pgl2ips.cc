
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/gadgets.h"
#include "xstd/h/iomanip.h"
#include "base/polyOpts.h"
#include "base/CmdLine.h"
#include "base/IpRange.h"
#include "pgl/AgentAddrIter.h"
#include "pgl/AgentSymIter.h"
#include "pgl/BenchSideSym.h"
#include "pgl/BenchSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglClonerSym.h"
#include "pgl/PglNetAddrParts.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/PglNetAddrRangeSym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglPp.h"
#include "pgl/PglStaticSemx.h"
#include "pgl/ProxySym.h"
#include "pgl/RobotSym.h"
#include "pgl/ServerSym.h"
#include "runtime/HostMap.h"

class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,    "help",          "list of options"),
			theVersOpt(this,    "version",       "package version info"),
			theCfgName(this,    "config <filename>",  "PGL configuration"),
			theCfgDirs(this,    "cfg_dirs <dirs>",  "directories for PGL #includes"),
			theAgent  (this,    "agent <agent>", "print addresses for the given agent only"),
			theHost   (this,    "host <host>",   "print addresses for the given host only"),
			theVar    (this,    "var <name>",    "print value of the specified variable")
			{}

		virtual bool validate() const;

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theCfgName;
		StrArrOpt theCfgDirs;
		StrOpt theAgent;
		StrOpt theHost;
		StrOpt theVar;
};

// Bench.*.host related information
class HostInfo {
public:

	HostInfo() {}
	HostInfo(const String &aName,
		const NetAddr &aHost,
		const Array<NetAddr> &addrs,
		const int start,
		const int count,
		const int anAgentCount);

	bool includes(const NetAddr &addr) const;
	ostream &print(ostream &os) const;

private:

	ostream &printIps(ostream &os) const;
	ostream &printVar(ostream &os, const String &var) const;

	String theName;
	NetAddr theHost;
	Array<IpRange> theAddrs;
	int theAgentCount;
};

// Agent (robot or server) related information
class AgentInfo {
public:

	AgentInfo() {}
	AgentInfo(const String &aName,
		const String &aKind,
		const ArraySym *addrs);

	const Array<NetAddr> &addrs() const { return theAddrs; }
	int totalAgentCount() const { return theAddrs.count() * theAgentsPerAddr; }
	void calcRanges();
	ostream &print(ostream &os) const;

private:

	ostream &printIps(ostream &os) const;
	ostream &printVar(ostream &os, const String &var) const;
	int agentCount() const;

	String theName;
	String theKind;
	Array<NetAddr> theAddrs;
	Array<IpRange> theRanges;
	int theAgentsPerAddr;
};


static MyOpts TheOpts;

static PtrArray<HostInfo*> TheCltHosts;
static PtrArray<HostInfo*> TheSrvHosts;

static PtrArray<AgentInfo*> TheRobotAgents;
static PtrArray<AgentInfo*> TheServerAgents;

static const HostInfo *TheHostScope(0);


/* MyOpt */

bool MyOpts::validate() const {
	if (!theCfgName)
		cerr << "must specify PGL configuration file (--config)" << endl;
	else
		return true;
	return false;
}


/* HostInfo */

HostInfo::HostInfo(const String &aName,
		const NetAddr &aHost,
		const Array<NetAddr> &addrs,
		const int start,
		const int count,
		const int anAgentCount):
	theName(aName),
	theHost(aHost),
	theAddrs(count),
	theAgentCount(anAgentCount) {
	Assert(0 <= start);
	Assert(0 <= count);
	Assert(start + count <= addrs.count());
	Array<NetAddr> hostAddrs(count);
	for (int i = start; i < start + count; ++i)
		hostAddrs.append(addrs[i]);
	CompactAllAddrs(hostAddrs, theAddrs);
}

bool HostInfo::includes(const NetAddr &addr) const {
	for (int i = 0; i < theAddrs.count(); ++i)
		if (theAddrs[i].includes(addr))
			return true;
	return false;
}

ostream &HostInfo::print(ostream &os) const {
	if (TheOpts.theVar) {
		if (TheOpts.theVar.startsWith(theName + '.')) {
			const String var(TheOpts.theVar(theName.len() + 1, TheOpts.theVar.len()));
			printVar(os, var);
		}
		return os;
	}

	os << theName << ": host=" << theHost << " ips=";
	printIps(os) << " agents=" << theAgentCount;
	return os << endl;
}

ostream &HostInfo::printIps(ostream &os) const {
	if (theAddrs.empty())
		os << "none";
	else
		for (int i = 0; i < theAddrs.count(); ++i) {
			if (i)
				os << ',';
			theAddrs[i].print(os);
		}
	return os;
}

ostream &HostInfo::printVar(ostream &os, const String &var) const {
	static const String host("host");
	static const String ips("ips");
	static const String agents("agents");
	if (var == host)
		os << theHost << endl;
	else
	if (var == ips)
		printIps(os) << endl;
	else
	if (var == agents)
		os << theAgentCount << endl;
	else
		cerr << "Unknown variable '" << var << "'." << endl;

	return os;
}


/* AgentInfo */

AgentInfo::AgentInfo(const String &aName,
	const String &aKind,
	const ArraySym *addrs):
	theName(aName),
	theKind(aKind) {
	if (addrs) {
		HostMap seen(addrs->count()); // lookup table to weed out duplicates
		for (int i = 0; i < addrs->count(); ++i) {
			const NetAddrSym &addr_sym =
				(const NetAddrSym&)addrs->item(i)->cast(NetAddrSym::TheType);
			const NetAddr addr(addr_sym.val().addrN(), -1); // we only care about the IP
			int idx = -1;
			if (!seen.find(addr, idx)) {
				theAddrs.append(addr);
				(void)seen.addAt(idx, addr);
			}
		}
		theAgentsPerAddr = addrs->count() / theAddrs.count();
		if (theAgentsPerAddr * theAddrs.count() != addrs->count())
			cerr << "warning: " << theName << " total address count "
				<< "is not divisible by the unique address count, "
				<< "agents per host counters will be inaccurate"
				<< endl;
	} else
		theAgentsPerAddr = 0;
}

void AgentInfo::calcRanges() {
	Assert(theRanges.empty());
	Array<NetAddr> addrs;
	if (TheHostScope) {
		addrs.stretch(theAddrs.count());
		for (int i = 0; i < theAddrs.count(); ++i)
			if (TheHostScope->includes(theAddrs[i]))
				addrs.append(theAddrs[i]);
	}
	CompactAllAddrs(TheHostScope ? addrs : theAddrs, theRanges);
}

ostream &AgentInfo::print(ostream &os) const {
	if (theRanges.empty())
		return os;

	if (TheOpts.theVar) {
		if (TheOpts.theVar.startsWith(theName + '.')) {
			const String var(TheOpts.theVar(theName.len() + 1, TheOpts.theVar.len()));
			printVar(os, var);
		}
		return os;
	}

	os << theName << ": kind=" << theKind << " ips=";
	printIps(os) << " agents=" << agentCount();
	return os << endl;
}

ostream &AgentInfo::printIps(ostream &os) const {
	if (theRanges.empty())
		os << "none";
	else
		for (int i = 0; i < theRanges.count(); ++i) {
			if (i)
				os << ',';
			theRanges[i].print(os);
		}
	return os;
}

ostream &AgentInfo::printVar(ostream &os, const String &var) const {
	static const String kind("kind");
	static const String ips("ips");
	static const String agents("agents");
	if (var == kind)
		os << theKind << endl;
	else
	if (var == ips)
		printIps(os) << endl;
	else
	if (var == agents)
		os << agentCount() << endl;
	else
		cerr << "Unknown variable '" << var << "'." << endl;

	return os;
}

int AgentInfo::agentCount() const {
	int count(0);
	for (int i = 0; i < theRanges.count(); ++i)
		count += theRanges[i].count() * theAgentsPerAddr;
	return count;
}


// get unique addresses from AgentInfo array
static void getAgentAddrs(const Array<AgentInfo*> &agentInfos, Array<NetAddr> &addrs, int &totalAgentCount) {
	// count all addresses to size the lookup table
	totalAgentCount = 0;
	for (int i = 0; i < agentInfos.count(); ++i)
		totalAgentCount += agentInfos[i]->totalAgentCount();

	HostMap seen(totalAgentCount); // lookup table to weed out duplicates

	// append unique IP addresses only
	for (int i = 0; i < agentInfos.count(); ++i) {
		const AgentInfo &agentInfo(*agentInfos[i]);
		for (int j = 0; j < agentInfo.addrs().count(); ++j) {
			const NetAddr addr(agentInfo.addrs()[j]);
			int idx = -1;
			if (!seen.find(addr, idx)) {
				addrs.append(addr);
				(void)seen.addAt(idx, addr);
			}
		}
	}
}

// make host/agent name by appending a number to given string
static String makeName(const String &base, const int n) {
	static char buf[256];
	ofixedstream ofs(buf, sizeof(buf)/sizeof(*buf));
	ofs << base << n << ends;
	buf[sizeof(buf)/sizeof(*buf) - 1] = '\0';
	return String(buf);
}

// collect host info
static void configureHosts(const String &nameBase,
	const String &agentType,
	const Array<AgentInfo*> &agentInfos,
	const BenchSideSym &benchSide,
	Array<HostInfo*> &hostInfos) {
	if (TheHostScope)
		return;

	PtrArray<NetAddr *> hosts;
	benchSide.hosts(hosts);
	if (hosts.empty())
		clog << "no real host addresses for " << agentType
			<< " side specified" << endl << xexit;

	Array<NetAddr> addrs;
        int totalAgentCount;
	getAgentAddrs(agentInfos, addrs, totalAgentCount);
	if (!addrs.count())
		return;

	const int agentsPerHost = totalAgentCount / hosts.count();
	const int addrsPerHost = addrs.count() / hosts.count();
	if (addrsPerHost * hosts.count() != addrs.count())
		clog <<
			"the number of agent addresses (" << addrs.count() <<
			") is not divisible by the number of real host "
			"addresess (" << hosts.count() << ')' << endl << xexit;

	for (int i = 0; i < hosts.count(); ++i) {
		const String name(makeName(nameBase, i+1));
		if (!TheOpts.theHost ||
			name == TheOpts.theHost ||
			hosts[i]->addrA() == TheOpts.theHost) {
			HostInfo *const info = new HostInfo(name, *hosts[i],
				addrs, i*addrsPerHost, addrsPerHost,
				agentsPerHost);
			hostInfos.append(info);
			if (TheOpts.theHost) {
				TheHostScope = info;
				break;
			}
		}
	}
}

// collect agent info
static void configureAgents(const String &nameBase, const String &agentType, Array<AgentInfo*> &agentInfos) {
	static AgentSymIter::Agents &agents = PglStaticSemx::TheAgentsToUse;

	int n(1);
	for (AgentSymIter i(agents, agentType, false); i; ++i) {
		const String name(makeName(nameBase, n++));
		const AgentSym &agent(*i.agent());
		if (!TheOpts.theAgent ||
			name == TheOpts.theAgent ||
			agent.kind() == TheOpts.theAgent) {
			AgentInfo *const info = new AgentInfo (name,
				agent.kind(), agent.addresses());
			agentInfos.append(info);
		}
	}
}

// print current scope
static ostream &printScope(ostream &os) {
	os << "scope: host=";
	if (TheOpts.theHost)
		TheOpts.theHost.report(os);
	else
		os << '*';
	os << " agent=";
	if (TheOpts.theAgent)
		TheOpts.theAgent.report(os);
	else
		os << '*';
	return os << endl;
}


int main(int argc, char **argv) {
	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate())
		return -1;

	configureStream(cout, 2);
	configureStream(clog, 3);

	TheOpts.theCfgDirs.copy(PglPp::TheDirs);
	PglStaticSemx::Interpret(TheOpts.theCfgName);

	if (!PglStaticSemx::TheBench)
		clog << "no bench selected with use()" << endl << xexit;

	if (!TheOpts.theVar)
		printScope(cout) << endl;

	configureAgents("robot", RobotSym::TheType, TheRobotAgents);
	configureAgents("server", ServerSym::TheType, TheServerAgents);
	if (TheRobotAgents.empty() && TheServerAgents.empty())
		cerr << "No" << (TheOpts.theAgent ? "matching " : " ")
			<< "agents found" << endl << xexit;

	configureHosts("polyclt", RobotSym::TheType, TheRobotAgents, *PglStaticSemx::TheBench->clientSide(), TheCltHosts);
	configureHosts("polysrv", ServerSym::TheType, TheServerAgents, *PglStaticSemx::TheBench->serverSide(), TheSrvHosts);
	if (TheCltHosts.empty() && TheSrvHosts.empty())
		cerr << "No" << (TheOpts.theHost ? "matching " : " ")
			<< "hosts found" << endl << xexit;

	ostream::pos_type p(cout.tellp());
	for (int i = 0; i < TheCltHosts.count(); ++i)
		TheCltHosts[i]->print(cout);
	for (int i = 0; i < TheSrvHosts.count(); ++i)
		TheSrvHosts[i]->print(cout);
	if (cout.tellp() != p)
		cout << endl;

	for (int i = 0; i < TheRobotAgents.count(); ++i) {
		TheRobotAgents[i]->calcRanges();
		TheRobotAgents[i]->print(cout);
	}
	for (int i = 0; i < TheServerAgents.count(); ++i) {
		TheServerAgents[i]->calcRanges();
		TheServerAgents[i]->print(cout);
	}

	return 0;
}
