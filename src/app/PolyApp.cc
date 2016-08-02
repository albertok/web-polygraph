
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/signal.h"

#include <fstream>
#include <memory>

#include "xstd/h/new.h"
#include "xstd/h/fcntl.h"
#include "xstd/h/net/if.h"
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"
#include "xstd/h/os_std.h"
#include "xstd/h/process.h"  /* for _getpid() on W2K */
#include "xstd/h/sys/stat.h"

#include "xstd/Socket.h"
#include "xstd/AlarmClock.h"
#include "xstd/CpuAffinitySet.h"
#include "xstd/FileScanner.h"
#include "xstd/ResourceUsage.h"
#include "xstd/Rnd.h"
#include "xstd/gadgets.h"
#include "xstd/getIfAddrs.h"
#include "xstd/InetIfReq.h"
#include "xstd/NetIface.h"
#include "xstd/rndDistrs.h"
#include "base/RndPermut.h"
#include "base/UniqId.h"
#include "base/BStream.h"
#include "base/IpRange.h"
#include "base/polyLogTags.h"
#include "base/polyLogCats.h"
#include "base/PatchRegistry.h"
#include "runtime/AddrMap.h"
#include "runtime/HostMap.h"
#include "runtime/AddrSubsts.h"
#include "runtime/HttpDate.h"
#include "runtime/StatCycle.h"
#include "runtime/StatPhase.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/StatsSampleMgr.h"
#include "runtime/ErrorMgr.h"
#include "runtime/Notifier.h"
#include "runtime/PolyOLog.h"
#include "runtime/LogComment.h"
#include "runtime/SharedOpts.h"
#include "runtime/Agent.h"
#include "runtime/AgentCfg.h"
#include "runtime/PersistWorkSetMgr.h"
#include "runtime/httpHdrs.h"
#include "runtime/FtpMsg.h"
#include "runtime/BcastSender.h"
#include "runtime/polyErrors.h"
#include "runtime/polyBcastChannels.h"
#include "runtime/globals.h"
#include "csm/ContentMgr.h"
#include "csm/ContentSel.h"
#include "pgl/PglPp.h"
#include "pgl/PglParser.h"
#include "pgl/PglStaticSemx.h"
#include "pgl/PglCtx.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/AgentSym.h"
#include "pgl/AgentAddrIter.h"
#include "pgl/AgentSymIter.h"
#include "pgl/RobotSym.h"
#include "pgl/ServerSym.h"
#include "pgl/PhaseSym.h"
#include "pgl/BenchSym.h"
#include "pgl/BenchSideSym.h"
#include "pgl/ProxySym.h"
#include "runtime/ProxyCfg.h"
#include "app/DebugSwitch.h"
#include "app/PolyApp.h"
#include "app/BeepDoorman.h"
#include "app/shutdown.h"
#include "app/WorkersRunner.h"
#include "server/SrvCfg.h"


static StatCycle *TheStatCycle = 0;

#if FIND_MEMORY_LEAKS
static void DebugMemSignal(int s);
#endif

/* PolyApp */

PolyApp::PolyApp(): theBeepDoorman(0),
	isIdle(false), theStateCount(0), theWorkerCount(0) {
	theChannels.append(ThePhasesEndChannel);
}

PolyApp::~PolyApp() {
	for (int i = 0; i < theLocals.count(); ++i)
		delete theLocals[i];
	Socket::Clean();
	delete theBeepDoorman;
}

// scans all FDs once
void PolyApp::step() {
	Time tout = isIdle ? theIdleEnd - TheClock : (Time)TheOpts.theIdleTout;
	Time *toutp = &tout;

	Clock::Update(false);

	if (!TheAlarmClock.on()) {
		if (tout < 0)
			toutp = 0;
	} else
	if (tout < 0) {
		tout = TheAlarmClock.timeLeft();
	} else {
		tout = Min(tout, TheAlarmClock.timeLeft());
	}

	// avoid multiple micro iterations when alarm time is very close
	if (tout > 0)
		tout = Max(Time::Msec(1), tout);
	checkTiming(tout);

	const int readyCount = scan(toutp);

	if (ShutdownRequested())
		return;

	if (readyCount < 0) {
		if (const Error err = Error::LastExcept(EINTR))
			FatalError(err);
	}

	// start "idle state" if needed
	if (!readyCount) {
		if (!isIdle && TheOpts.theIdleTout >= 0) {
			isIdle = true;
			theIdleBeg = TheClock;
			theIdleEnd = theIdleBeg + TheOpts.theIdleTout;
		}
	} else {
		isIdle = false;
	}

	Clock::Update();

	checkProgressReport();

	if (!readyCount && isIdle && TheClock.time() >= theIdleEnd)
		wasIdle(TheClock - theIdleBeg);
}

int PolyApp::scan(Time *toutp) {
	static int scanCount = 0;
	static Time zeroTout = Time::Sec(0);

	const int hotCount = scanCount++ % TheOpts.thePrioritySched == 0 ?
		TheFileScanner->scan(toutp) :
		TheFileScanner->scan(fsupAsap, &zeroTout);

	if (!hotCount)
		scanCount = 0;

	return hotCount;
}

void PolyApp::begCycle(int) {
	Clock::Update(false);
	theTickCount = 0;
}

void PolyApp::endCycle() {
}

bool PolyApp::tick() {
	if (++theTickCount % 8 == 0)
		Clock::Update(false);
	return !ShutdownRequested();
}

void PolyApp::checkTiming(Time drift) {
	// make these members
	static Time nextDriftToReport = Time::Msec(100);
	static Time lastCheck = TheClock;
	if (drift < 0) { // we are behind
		drift = -drift;
		if (drift >= nextDriftToReport) {
			if (ReportError(errTimingDrift))
				Comment(1) << "record level of timing drift: " << drift 
					<< "; last check was " << (TheClock-lastCheck) << " ago" << endc;
			nextDriftToReport = drift + drift/5;
		}
	}
	lastCheck = TheClock;
}

void PolyApp::checkProgressReport() const {
    const Time reportGap = Time::Sec(5*60);
    static Time lastReport = TheClock;
    if (lastReport + reportGap <= TheClock) {
		lastReport = TheClock;
		reportRUsage();
		Broadcast(TheInfoChannel, BcastRcver::ieReportProgress);
	}
}

void PolyApp::noteMsgStrEvent(BcastChannel *ch, const char *msg) {
	Assert(ch == ThePhasesEndChannel);
	// quit now unless we need to wait for inactivity timeout
	if (TheOpts.theIdleTout < 0 || TheStatPhaseMgr.reachedNegative())
		ShutdownReason(msg);
}

void PolyApp::wasIdle(Time tout) {
	if (TheOpts.theIdleTout >= 0 && tout >= TheOpts.theIdleTout) {
		Comment << "was idle for at least " << tout << endc;
		ShutdownReason("inactivity timeout");
	}
}

bool PolyApp::handleCmdLine(int argc, char *argv[]) {

	Array<OptGrp*> opts;
	getOpts(opts);

	theCmdLine.configure(opts);
	if (!theCmdLine.parse(argc, argv))
		return false;

	// validate command line params
	for (int i = 0; i < opts.count(); ++i)
		if (!opts[i]->validate())
			return false;

	if (!TheOpts.theLclRngSeed.wasSet() && TheOpts.theWorkerId.wasSet())
		TheOpts.theLclRngSeed.set(TheOpts.theWorkerId);

	return true;
}

void PolyApp::configureBinLog(OLog &log, const String &fname, SizeOpt &bufSz) {
	if (ostream *os = new ofstream(fname.cstr(), ios::binary|ios::out|ios::trunc)) {
		log.stream(fname, os);
	} else {
		cerr << ThePrgName << ": cannot create binary log; " 
			<< fname << ": " << Error::Last() << endl << xexit;
	}

	// we cannot shrink log size
	if (bufSz > log.capacity())
		log.capacity(bufSz); 
	bufSz.set(log.capacity());
}

void PolyApp::configureLogs(int prec) {
	// redirect console output
	if (TheOpts.theConsFileName && TheOpts.theConsFileName != "-")
		redirectOutput(TheOpts.theConsFileName.cstr());

	configureStream(cout, prec);
	configureStream(cerr, prec);
	configureStream(clog, prec);

	/* initialize binary log files */

	if (TheOpts.theLogFileName) {
		configureBinLog(TheOLog, TheOpts.theLogFileName, TheOpts.theLogBufSize);
		TheOLog.period(Time::Sec(5)); // flush at most that often
	}
	
	if (TheOpts.theSmplLogFileName) {
		TheSmplOLog = new PolyOLog();
		configureBinLog(*TheSmplOLog, TheOpts.theSmplLogFileName, TheOpts.theSmplLogBufSize);
		// sample log is not flushed on a periodic basis
	} else {
		// use general purpose log
		TheOpts.theSmplLogFileName.val(TheOpts.theLogFileName);
		TheOpts.theSmplLogBufSize.set(TheOpts.theLogBufSize);
		TheSmplOLog = &TheOLog;
	}

	Comment.TheEchoLevel = TheOpts.theVerbLevel;
}

void PolyApp::configureRnd() {
	ThePersistWorkSetMgr.loadSeeds(); // must be called before seeds are used

	// set random seeds
	GlbPermut().reseed(TheOpts.theGlbRngSeed);
	LclPermut().reseed(TheOpts.theLclRngSeed);
	RndGen::DefSeed(LclPermut(TheOpts.theLclRngSeed));

	// use the seed as uid "space" index for non-unique worlds
	if (!TheOpts.useUniqueWorld)
		UniqId::Space(TheOpts.theLclRngSeed);
	TheGroupId = UniqId::Create();

	ThePersistWorkSetMgr.configure(); // must be called after UniqId::Space()
}

// collect server configurations into TheHostMap
void PolyApp::configureHosts() {

	TheAddrMap = new AddrMap();
	TheAddrMap->configure(PglStaticSemx::TheAddrMapsToUse);

	TheAddrSubsts = new AddrSubsts;
	TheAddrSubsts->configure(PglStaticSemx::TheAddrSubstsToUse);

	AgentSymIter::Agents &agents = PglStaticSemx::TheAgentsToUse;

	// count the global number of servers for HostMap
	// include AddrMap names in the count as we add them below
	int srvCount = TheAddrMap->nameCount();
	for (AgentSymIter i(agents, ServerSym::TheType, true); i; ++i)
		srvCount += i.agent()->hostCount();

	TheHostMap = new HostMap(srvCount);

	// create selectors for each server configuration symbol
	// note: we stretch for all agents, not just servers
	// preset with nil pointers: proxies and robots have no ContentSel
	theContentSels.resize(agents.count());
	for (AgentSymIter i(agents, ServerSym::TheType, false); i; ++i) {
		ContentSel *sel = new ContentSel;
		sel->configure(static_cast<const ServerSym*>(i.agent())); // XXX:cast()
		theContentSels.put(sel, i.agentIdx());
	}

	// get client-side ContentCfg added
	for (AgentSymIter i(agents, RobotSym::TheType, false); i; ++i) {
		const RobotSym *const clt =
			&static_cast<const RobotSym&>(i.agent()->cast(RobotSym::TheType));
		configureRobotContent(*clt, "post_contents");
		configureRobotContent(*clt, "put_contents");
	}

	// assign content selectors to origin servers
	// assign protocols and SSL wraps to servers, including proxies
	int sslServerCount = 0;
	Array<int> forAddrMap(srvCount);
	for (AgentAddrIter i(agents, ServerSym::TheType); i; ++i) {
		int idx = -1;
		const NetAddr &host = i.address();

		if (host.isDomainName()) {
			cerr << here << "server address must be an IP, got: " <<
				host << endl << xexit;
		}

		HostCfg *hostCfg = addToHostMap(host, idx);

		if (ContentSel *sel = theContentSels[i.agentIdx()])
			hostCfg->theContent = sel;

		// will add host addresses by default, but do not mix find with add
		if (!TheAddrMap->findAddr(host))
			forAddrMap.append(idx);

		const ServerSym *srv =
			&static_cast<const ServerSym&>(i.agent()->cast(ServerSym::TheType));
		SrvCfg *const srvCfg = TheSrvSharedCfgs.getConfig(srv);
		hostCfg->theProtocol =
			static_cast< Agent::Protocol >(srvCfg->selectProtocol());
		if (srvCfg->sslActive(hostCfg->theProtocol)) {
			hostCfg->isSslActive = true;

			const int fyiLimit = 25;
			++sslServerCount;
			if (sslServerCount < fyiLimit) {
				Comment(6) << "fyi: expecting SSL proxy or server at " << 
					host << endc;
			}
			if (sslServerCount == fyiLimit) {
				Comment(6) << "fyi: suppressing further reports about " <<
					"expected SSL proxies and servers" << endc;
			}
		}
	}

	if (sslServerCount) {
		Comment(3) << "fyi: expecting " << sslServerCount << " SSL proxies " <<
				"and/or servers" << endc;
	}

	for (AgentAddrIter i(agents, ProxySym::TheType); i; ++i) {
		int idx = -1;
		const NetAddr &host = i.address();
		HostCfg *hostCfg = addToHostMap(host, idx);

		const ProxySym *proxy =
			&dynamic_cast<const ProxySym&>(i.agent()->cast(ProxySym::TheType));
		ProxyCfg *const proxyCfg = TheProxySharedCfgs.getConfig(proxy);
		if (proxyCfg->sslActive()) {
			hostCfg->theHostsBasedCfg = proxy;
			hostCfg->isSslActive = true;
		}
	}

	// add collected addresses to AddrMap
	for (int i = 0; i < forAddrMap.count(); ++i) {
		const int hostIdx = forAddrMap[i];
		const HostCfg *hostCfg = TheHostMap->at(hostIdx);
		Assert(hostCfg);
		TheAddrMap->add(hostCfg->theAddr);
	}

	// add all missing visible names; needed to get to public worlds
	for (int viserv = 0; viserv < TheAddrMap->nameCount(); ++viserv) {
		const NetAddr &viname = TheAddrMap->nameAt(viserv);
		int idx = -1;
		if (!TheHostMap->find(viname, idx))
			TheHostMap->addAt(idx, viname);
		// we do not create public world by default to save RAM if
		// it happens to be not-needed
	}
}

HostCfg *PolyApp::addToHostMap(const NetAddr &host, int &idx) {
	if (TheHostMap->find(host, idx)) {
		cerr << here << "server/proxy address " << host <<
			" repeated/use()d twice!" << endl << xexit;
	}

	return TheHostMap->addAt(idx, host);
}

void PolyApp::configureRobotContent(const RobotSym &clt, const String &fieldName) {
	Array<ContentSym*> syms;
	RndDistr *sel = 0;

	if (clt.reqContents(fieldName, syms, sel)) {
		Array<ContentCfg*> contents(syms.count());
		TheContentMgr.get(syms, contents);
	}

	delete sel;
}

// get a list of all available interface addresses
void PolyApp::getIfaces() {
	// manually configured [faked] addresses take priority
	if (TheOpts.theFakeHosts) {
		getFakeIfaces();
		return;
	}

	Array<InetIfReq> addrs;
	if (!::GetIfAddrs(addrs, String()))
		cerr << ThePrgName << ": cannot get a list of all available network interfaces: " << Error::Last() << endl << xexit;

	theIfaces.stretch(addrs.count());
	for (int i = 0; i < addrs.count(); ++i)
		theIfaces.append(addrs[i].addrN());
}

void PolyApp::getFakeIfaces() {
	// note each list item may be an IP range
	for (int i = 0; i < TheOpts.theFakeHosts.val().count(); ++i) {
		const String &item = *TheOpts.theFakeHosts.val()[i];
		PglNetAddrRange range;
		if (!range.parse(item))
			cerr << ThePrgName << ": cannot convert fake host `" << item 
				<< "' to an IP address or range" << endl << xexit;

		Array<NetAddr*> addrs;
		range.toAddrs(addrs);
		for (int a = 0; a < addrs.count(); ++a)
			theIfaces.append(addrs[a]->addrN());
		while (addrs.count()) delete addrs.pop();
	}
}

// creates IP addresses for agents to bind to
void PolyApp::makeAddresses() {
	PtrArray<NetAddr*> hosts; // global host addresses
	PtrArray<NetAddrSym*> agents; // global agent addresses needing IP aliases
	getHostAddrs(hosts);
	getAgentAliasAddrs(agents);

	ostringstream err;
	makeAddresses(hosts, agents, err);
	if (err.tellp()) {
		err << ends;
		Comment(5) << "fyi: " << err.str() 
			<< "; will not attempt to create agent addresses" << endc;
		streamFreeze(err, false);
	}
}

// hosts are all host addresses; agents are unique agent addresses
ostream &PolyApp::makeAddresses(Array<NetAddr*> &hosts, AddrSyms &agents, ostream &err) {
	if (!PglStaticSemx::TheBench)
		return err << "no bench selected with use()";

	if (!hosts.count())
		return err << "no real host addresses for " << theAgentType << " side specified";

	if (!agents.count())
		return err << "no " << theAgentType << " addresses (with an interface name) found";

	const int agentsPerHost = agents.count() / hosts.count();
	if (agentsPerHost * hosts.count() != agents.count())
		return err <<
			"the number of virtual agent addresses with interface names (" <<
			agents.count() << ") is not divisible by the number of real " <<
			"host addresess (" << hosts.count() << ')';

	getIfaces();

	// create aliases for each local host address
	int createCount = 0;
	AliasIndexes aliasIndexes;

	for (int h = 0; h < hosts.count(); ++h) {
		const NetAddr &host = *hosts[h];

		// check if host address is local
		bool found = false;
		for (int l = 0; !found && l < theIfaces.count(); ++l)
			found = theIfaces[l] == host.addrN();

		if (found)
			createCount += makeAddresses(h, agents, agentsPerHost, aliasIndexes);
	}

	if (!createCount) {
		cerr << ThePrgName << ": no specified host addresses match local addresses" << endl;

		cerr << hosts.count() << " host addresses:";
		for (int h = 0; h < hosts.count(); ++h)
			cerr << ' ' << *hosts[h];
		cerr << endl;

		cerr << theIfaces.count() << " local addresses: ";
		for (int l = 0; l < theIfaces.count(); ++l)
			cerr << ' ' << NetAddr(theIfaces[l], -1);
		cerr << endl;

		cerr << xexit;
	}

	theIfaces.reset(); // no longer complete if we created aliases

	Comment(6) << "fyi: created " << createCount << " agent addresses total" << endc;

	// Give some time for aliases to be created, otherwise we may
	// get socket binding errors later.  Needed on FreeBSD at least.
	sleep(1);
	Clock::Update(false);

	return err;
}

int PolyApp::makeAddresses(int hidx, AddrSyms &agents, int agentsPerHost, AliasIndexes& aliasIndexes) {
	int aliasCount = 0;
	for (int i = hidx*agentsPerHost; i < agents.count() && aliasCount < agentsPerHost; ++i) {
		const String &ifName = agents[i]->ifName();
		const NetAddr &addr = agents[i]->val();

		++aliasCount;
		int subnet;
		Assert(agents[i]->subnet(subnet));
		const InAddress netmask = InAddress::NetMask(addr.addrN().family(), subnet);

		NetIface iface;
		iface.name(ifName);
		std::pair<AliasIndexes::iterator, bool> result =
			aliasIndexes.insert(std::make_pair(ifName, 0));
		int &aliasIndex = result.first->second;
		if (result.second) {
			// insertion took place, iface is seen for the first time
			if (TheOpts.deleteOldAliases)
				deleteAddresses(ifName);
			else
				aliasIndex = iface.nextAliasIndex();
		}
		if (!Should(iface.addAlias(addr.addrN(), aliasIndex++, netmask))) {
			agents[i]->print(cerr << "error: " << ifName
				<< ": failed to create new alias (", "")
				<< ')' << endl << xexit;
		}
	}
	Assert(aliasCount == agentsPerHost);
	return aliasCount;
}

// delete old aliases
void PolyApp::deleteAddresses(const String &ifname) {
	NetIface iface;
	iface.name(ifname);
	const int delCount = iface.delAliases();
	if (Should(delCount >= 0)) {
		Comment(1) << "fyi: " << ifname << ": deleted "
			<< delCount << " old IP aliases" << endc;
	} else {
		Comment << "error: " << ifname 
			<< ": failed to delete old IP aliases" << endc;
	}
}

void PolyApp::addUniqueAddrs(AddrSyms *const addrs, const NetAddrSym &s, HostMap &seen, Array<NetAddr> &skipped, HostMap &skipped_map) const {
	const NetAddr addr(s.val().addrN(), -1); // we only care about the IP
	int idx = -1;
	int subnet;
	if ((s.subnet(subnet) || s.val().addrN().family() == AF_INET6) &&
		s.ifName()) {
		if (!seen.find(addr, idx)) {
			if (addrs)
				addrs->append(&SymCast(NetAddrSym, *s.clone()));
			(void)seen.addAt(idx, addr);
                }
	} else if (addrs && !skipped_map.find(addr, idx)) {
		skipped_map.addAt(idx, addr);
		skipped.append(addr);
	}
}

void PolyApp::getHostAddrs(Array<NetAddr*> &hosts) const {
	if (PglStaticSemx::TheBench && PglStaticSemx::TheBench->side(sideName()))
		PglStaticSemx::TheBench->side(sideName())->hosts(hosts);
}

void PolyApp::getCpuCores(Array< Array<int> > &cpuCores) const {
	if (PglStaticSemx::TheBench && PglStaticSemx::TheBench->side(sideName()))
		PglStaticSemx::TheBench->side(sideName())->cpuCoresArray(cpuCores);
}

// Find unique IP addresses to avoid creation of duplicate aliases on different
// hosts when IPs are sliced: a,b,a,b must yield a|b IPs for a,a|b,b agents 
// Skip addresses without ifnames: addrs are used for alias creation only
void PolyApp::getAgentAliasAddrs(AddrSyms &addrs) const {
	Array<NetAddr> skipped;
	std::auto_ptr<HostMap> skipped_map; // lookup table to weed out duplicates
	Array<NetAddrSym *> bench_addrs;
	std::auto_ptr<HostMap> bench_addrs_map; // lookup table to weed out duplicates

	// use bench.side.addresses or collect from agents
	if (PglStaticSemx::TheBench &&
		PglStaticSemx::TheBench->side(sideName()) &&
		PglStaticSemx::TheBench->side(sideName())->addresses(bench_addrs)) {
		bench_addrs_map.reset(new HostMap(bench_addrs.count()));
		skipped_map.reset(new HostMap(bench_addrs.count()));
		// append unique IP addresses with ifnames only
		for (int i = 0; i < bench_addrs.count(); ++i) {
			addUniqueAddrs(&addrs, *bench_addrs[i], *bench_addrs_map,
				skipped, *skipped_map);
		}
	}

	// count all addresses to size the lookup table
	int allCount = 0;
	for (AgentSymIter i(PglStaticSemx::TheAgentsToUse, theAgentType, true); i; ++i)
		allCount += i.agent()->hostCount();

	// append unique IP addresses with ifnames only
	HostMap agent_addrs_map(allCount); // lookup table to weed out duplicates
	if (!skipped_map.get())
		skipped_map.reset(new HostMap(allCount));
	for (AgentAddrIter i(PglStaticSemx::TheAgentsToUse, theAgentType); i; ++i) {
		addUniqueAddrs(bench_addrs_map.get() ? 0 : &addrs,
			ConstSymCast(NetAddrSym, *i.addressSym()),
			agent_addrs_map, skipped, *skipped_map);
	}

	if (bench_addrs_map.get()) {
		allCount = bench_addrs.count();

		// compare bench.side.addresses and agent addresses
		const HostMap *a;
		HostMap *b; // HostMap::find() is not const
		String msg;
		if (bench_addrs_map->hostCount() > agent_addrs_map.hostCount()) {
			a = bench_addrs_map.get();
			b = &agent_addrs_map;
			msg = "bench.side.addresses address not used by any agent";
		} else {
			a = &agent_addrs_map;
			b = bench_addrs_map.get();
			msg = "agent address missing in bench.side.addresses";
		}
		for (int i = 0; i < a->iterationCount(); ++i) {
			const HostCfg *const host = a->at(i);
			if (host && !b->find(host->theAddr)) {
				Comment(5) << "warning: " << msg << ": " << host->theAddr << endc;
				break;
			}
		}
	}

	if (addrs.empty()) {
		Comment(6) << "fyi: all " << allCount << ' ' << theAgentType
			<< " IP address(es) lack interface name or netmask" << endc;
	} else if (!skipped.empty()) {
		Comment(6) << "fyi: these " << theAgentType
			<< " IP address(es) have no interface names or netmasks: ";
		Array<IpRange> ranges;
		CompactAllAddrs(skipped, ranges);
		const int limit = 100;
		for (int i = 0; i < Min(ranges.count(), limit); ++i) {
			if (i)
				Comment << ", ";
			Comment << ranges[i];
		}
		if (ranges.count() > limit)
			Comment << ", ...";
		Comment << endc;
	}
	Comment(3) << "found " << allCount << ' ' << theAgentType <<
		" IP address(es) total (" << addrs.count() <<
		" unique address(es) with interface name and netmask)." << endc;

	// cleanup
	while (bench_addrs.count()) delete bench_addrs.pop();
}

void PolyApp::makeAgents() {
	// PglCtx::RootCtx()->report(cout, "");

	// workers do not create aliases; the master process does that
	if (!TheOpts.theWorkerId.wasSet())
		makeAddresses();
	Clock::Update(false);

	getIfaces();

	// create and configure agents that are assigned to our host
	// configuration process may take a while; inform about the progress
	const Time cfgReportGap = Time::Sec(5); // how often to report
	const Time cfgStart = TheClock;
	Time cfgNextReport = cfgStart + cfgReportGap;

	// build lookup table with local host addresses 
	HostMap ifaceMap(theIfaces.count());
	for (int i = 0; i < theIfaces.count(); ++i) {
		const NetAddr addr(theIfaces[i], -1);
		int idx = -1;
		if (ifaceMap.find(addr, idx)) {
			Comment(1) << "warning: multiple interfaces have address " <<
				addr << endc;
			continue;
		}
		// TODO: optimize so that we do not have to create unused HostCfgs
		(void)ifaceMap.addAt(idx, addr);
	}

	int agentCount = 0; // global
	Array<const AgentSym*> localAgents;
	PtrArray<const NetAddr*> localAgentAddreses;
	AgentSymIter::Agents &agents = PglStaticSemx::TheAgentsToUse;
	for (AgentAddrIter i(agents, theAgentType); i; ++i) {
		++agentCount;
		const NetAddr addr(i.address().addrN(), -1);
		if (ifaceMap.find(addr)) {
			localAgents.append(i.agent());
			localAgentAddreses.append(new NetAddr(i.address()) );
		}
	}

	PtrArray<NetAddr*> hosts; // global host addresses
	getHostAddrs(hosts);
	if (!hosts.empty() && (agentCount % hosts.count())) {
		Comment(5) << "warning: the number of agent addresses (" <<
			agentCount << ") is not divisible by the number of real " <<
			"host addresess (" << hosts.count() << ')'<< endc;
	}

	Array< Array<int> > cpuCores;
	getCpuCores(cpuCores);

	Array< Array<int> > localCpuCores;
	int localHostCount = 0;
	for (int h = 0; h < hosts.count(); ++h) {
		const NetAddr &host = *hosts[h];
		if (ifaceMap.find(host)) {
			++localHostCount;
			if (cpuCores.count() > h)
				localCpuCores.append(cpuCores[h]);
		}
	}

	// do not start workers without agents, even if there are cores available
	const int workerCount = min(localAgents.count(), localHostCount);

	if (workerCount > 1 && !TheOpts.theWorkerId.wasSet()) {
		// check and warn about the worker load problem in the master process
		if (const int busierWorkers = localAgents.count() % localHostCount) {
			Comment(5) << "warning: the number of local agent addresses (" <<
				localAgents.count() << ") is not divisible by the number " <<
				"of local host addresess (" << localHostCount << "); " <<
				"the first " << busierWorkers << " out of " << workerCount <<
				" workers will host an extra agent" << endc;
		}

		// the master process does not create agents; workers do that
		theWorkerCount = workerCount;
		return;
	}

	if (!cpuCores.empty() && cpuCores.count() != hosts.count()) {
		Comment << "error: number of Bench cpu_cores (" << cpuCores.count() <<
			") differs from the number of Bench hosts (" << hosts.count() <<
			")" << endc << xexit;
	}

	int agentBegin = 0; // the first agent that belongs to this worker
	int agentEnd = 0; // the first agent that belongs to the next worker
	if (workerCount <= 1)
		agentEnd = localAgents.count();
	else
	if (1 <= TheOpts.theWorkerId && TheOpts.theWorkerId <= workerCount) {
		// all hosts will get at least that many agents (could be zero)
		const int agentsPerHost = localAgents.count() / workerCount;
		// that many hosts will get a single extra agent (could be zero)
		const int hostsWithExtra = localAgents.count() % workerCount;
		const bool thisHostNeedsExtra = TheOpts.theWorkerId <= hostsWithExtra;
		const int hostsToTheLeft = TheOpts.theWorkerId - 1;
		agentBegin = thisHostNeedsExtra ?
			(hostsToTheLeft * (agentsPerHost + 1)) :
			(hostsToTheLeft * agentsPerHost + hostsWithExtra);
		agentEnd = agentBegin + agentsPerHost + (thisHostNeedsExtra ? 1 : 0);
	} else {
		Comment << "error: --worker value " << TheOpts.theWorkerId <<
			" exceeds the number of local agents " << localAgents.count() <<
			"; no agents will be started for this worker" << endc;
	}

	for (int agentIdx = agentBegin; agentIdx < agentEnd; ++agentIdx) {
		makeAgent(*localAgents[agentIdx], *localAgentAddreses[agentIdx]);
		Clock::Update(false);
		if (TheClock.time() >= cfgNextReport) {
			Comment(5) << "created " << setw(6) << theLocals.count() <<
				" agents so far" << endc;
			cfgNextReport = TheClock + cfgReportGap; // drift is OK
		}
	}

	Clock::Update(false);
	Comment(4) << "created " << theLocals.count() << " agents total" << endc;

	if (!theLocals.count()) {
		cerr << ThePrgName << ": no " << theAgentType 
			<< " matches local interface addresses" << endl;

		ArraySym agentAddrs("addr");
		{for (AgentSymIter i(PglStaticSemx::TheAgentsToUse, theAgentType, false); i; ++i)
			i.agent()->addresses(agentAddrs);
		}

		ArraySym localAddrs("addr");
		localAddrs.reserve(theIfaces.count());
		{for (int i = 0; i < theIfaces.count(); ++i) {
			NetAddrSym s;
			s.val(NetAddr(theIfaces[i], -1));
			localAddrs.add(s);
		}}

		agentAddrs.print(cerr << agentAddrs.count() << ' ' << theAgentType << " addresses:", "");
		cerr << endl;

		localAddrs.print(cerr << localAddrs.count() << " local addresses: ", "");
		cerr << endl;

		cerr << xexit;
	}

	if (localCpuCores.count()) {
		const int cpuCoresIndex = TheOpts.theWorkerId.wasSet() ? TheOpts.theWorkerId - 1 : 0;
		if (cpuCoresIndex < 0 || cpuCoresIndex >= localCpuCores.count()) {
			Comment(1) << "warning: --worker value " << TheOpts.theWorkerId <<
				" exceeds the number of local hosts " << localCpuCores.count() <<
				"; ignoring CPU affinity for this worker" << endc;
		} else if (localCpuCores[cpuCoresIndex].count()) {
			CpuAffinitySet cpuAffinitySet;
			for (int i = 0; i < localCpuCores[cpuCoresIndex].count(); ++i)
				cpuAffinitySet.reset(localCpuCores[cpuCoresIndex][i], true);
			ostringstream err;
			const bool success = cpuAffinitySet.apply(err);
			if (!success || err.tellp()) {
				const char *label = success ? "warning: " : "error: ";
				Comment(1) << label << "problems with setting CPU affinity";
				if (TheOpts.theWorkerId.wasSet())
					Comment << " for worker " << TheOpts.theWorkerId;
				Comment << ':' << std::endl << err.str() << endc;
				if (!success)
					Comment << xexit;
				streamFreeze(err, false);
			}
		}
	}

	theIfaces.reset();
}

void PolyApp::describeLocals() const {
	for (int i = 0; i < theLocals.count(); ++i) {
		theLocals[i]->describe(Comment(5) << theAgentType << ' ');
		Comment << endc;
	}
}

void PolyApp::addAgent(Agent *agent) {
	Assert(agent);
	theLocals.append(agent);
}

// configure StatPhaseMgr with user specified phases
void PolyApp::buildSchedule() {
	if (!PglStaticSemx::TheSchedule.count()) {
		cerr << ThePrgName << ": warning: no run phases were specified; generating and using default phase" << endl;
		PhaseSym *ps = new PhaseSym();
		ps->loc(TokenLoc("-"));
		ps->name("dflt");
		PglStaticSemx::TheSchedule.append(ps);
	}

	const StatPhase *prevPh = 0;
	for (int i = 0; i < PglStaticSemx::TheSchedule.count(); ++i) {
		StatPhase *p = new StatPhase;
		p->configure(PglStaticSemx::TheSchedule[i], prevPh);
		TheStatPhaseMgr.addPhase(p);
		prevPh = p;
	}

	TheStatPhaseMgr.logCat(logCat());
}



// top level configure routine
void PolyApp::configure() {

	Socket::Configure();

	configureLogs(2);

	FtpReq::Configure();
	ReqHdr::Configure();
	RepHdr::Configure();

	startListen();

	// ignore some signals
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	TheFileScanner = TheOpts.theFileScanner.val();
	TheFileScanner->configure();
	TheFileScanner->ticker(this);

	// to be safe, cut about 3% from the actual limit
	const int fdLimit = TheFileScanner->fdLimit();
	Socket::TheMaxLevel = Max(0, fdLimit - 10 - fdLimit/33);
	// honor manually configured limit if it is lower
	if (TheOpts.theFDLimit >= 0 && TheOpts.theFDLimit < Socket::TheMaxLevel)
		Socket::TheMaxLevel = TheOpts.theFDLimit;
	TheOpts.theFDLimit.set(Socket::TheMaxLevel);

	TheContentMgr.configure();

	configureHosts();

	/* statistics */

	TheStatCycle = new StatCycle;
	TheStatCycle->period(TheOpts.theStatCycleLen);
	buildSchedule(); // configures TheStatPhaseMgr

	if (PglStaticSemx::TheSmplSchedule.count())
		TheStatsSampleMgr.configure(PglStaticSemx::TheSmplSchedule);

	// configure notification mechanism (old custom format)
	if (TheOpts.theNotifAddr) {
		Notifier *n = new Notifier(TheOpts.theRunLabel, TheOpts.theNotifAddr);
		if (n->active())
			TheStatCycle->notifier(n);
		else
			delete n;
	}

	// configure notification mechanism (beep)
	if (TheOpts.theBeepDoormanListAt || TheOpts.theBeepDoormanSendTo) {
		theBeepDoorman = new BeepDoorman;
		theBeepDoorman->configure(TheOpts.theBeepDoormanListAt, TheOpts.theBeepDoormanSendTo);
	}

	atexit(&ShutdownAtExit);
	Should(xset_new_handler(&ShutdownAtNew));
}

void PolyApp::getOpts(Array<OptGrp*> &opts) {
	opts.append(&TheOpts);
}

void PolyApp::parseConfigFile(const String &fname) {
	Assert(fname);
	PglStaticSemx::WorkerId(TheOpts.theWorkerId);
	TheOpts.theCfgDirs.copy(PglPp::TheDirs);
	// save PGL configuration to log it later
	thePglCfg = PglStaticSemx::Interpret(fname);
}

void PolyApp::reportRUsage() const {
	ResourceUsage ru = ResourceUsage::Current();
	Comment(5) << "resource usage: " << endl;
	ru.report(Comment, "\t");
	Comment << endc;
}

void PolyApp::reportCfg() {

	theCmdLine.reportRaw(Comment(1)); Comment << endc;
	theCmdLine.reportParsed(Comment(2)); Comment << endc;

	if (CountPatches()) {
		ReportPatches(Comment(0));
		Comment << endc;
	}

	// report server configs
	Comment(7) << "Server content distributions:" << endl;
	for (int i = 0; i < theContentSels.count(); ++i) {
		if (theContentSels[i])
			theContentSels[i]->reportCfg(Comment);
	}
	Comment << endc;

	TheStatPhaseMgr.reportCfg(Comment(2) << "Phases:" << endl);
	Comment << endc;
	TheStatsSampleMgr.reportCfg(Comment(2) << "StatsSamples:" << endl);
	Comment << endc;

	Comment(2) << "FDs: "
		<< GetCurFD() << " out of " << GetMaxFD()
		<< " FDs can be used; safeguard limit: " << Socket::TheMaxLevel 
		<< endc;

	reportRUsage();

	Comment(1) << "group-id: " << TheGroupId << " pid: " << getpid() << endc;
	TheOLog << bege(lgGroupId, lgcAll) << TheGroupId << ende;

	Clock::Update(false);
	Comment << "current time: " << TheClock.time() << " or ";
	HttpDatePrint(Comment) << endc;
}

void PolyApp::logCfg() {
	// PGL configuration
	TheOLog << bege(lgPglCfg, lgcAll) << thePglCfg << ende;
	Comment(5) << "fyi: PGL configuration stored (" << thePglCfg.len() << "bytes)" << endc;
}

void PolyApp::logGlobals() {
	TheOLog << bege(lgContTypeKinds, lgcAll);
	ContType::Store(TheOLog);
	TheOLog << ende;
}

void PolyApp::flushState() {
	theStateCount++;
	logState(TheOLog);
	Comment(5) << "fyi: current state (" << theStateCount << ") stored" << endc;
}

// except for random seeds
void PolyApp::loadPersistence() {
	ThePersistWorkSetMgr.loadUniverses();

	// load per-agent data like private worlds
	if (IBStream *is = ThePersistWorkSetMgr.loadSideState()) {
		const int storedCount = is->geti();
		ThePersistWorkSetMgr.checkInput();

		if (storedCount != theLocals.count()) {
			Comment(1) << "warning: persistent working set stored in " <<
				TheOpts.doLoadWorkSet << " has information about " <<
				storedCount << ' ' << theAgentType << " agent(s) while " <<
				"the current test has " << theLocals.count() << " agent(s)" <<
				endc;
		}

		for (int i = 0; i < theLocals.count() && i < storedCount; ++i) {
			if (i < storedCount)
				theLocals[i]->loadWorkingSet(*is);
			else
				theLocals[i]->missWorkingSet();
		}

		ThePersistWorkSetMgr.checkInput();
		Comment(1) << "fyi: working set loaded from " << 
			is->name() << "; id: " << ThePersistWorkSetMgr.id() << 
			", version: " << ThePersistWorkSetMgr.version() << endc;
	}
}

// except for random seeds
void PolyApp::storePersistence() {
	ThePersistWorkSetMgr.storeUniverses();
	// store per-agent data like private worlds
	if (OBStream *os = ThePersistWorkSetMgr.storeSideState()) {
		*os << theLocals.count();
		for (int i = 0; i < theLocals.count(); ++i)
			theLocals[i]->storeWorkingSet(*os);

		ThePersistWorkSetMgr.checkOutput();
		Comment(1) << "fyi: working set stored in " << 
			os->name() << "; id: " << ThePersistWorkSetMgr.id() << 
			", version: " << ThePersistWorkSetMgr.version() << endc;
	}
}

void PolyApp::logState(OLog &log) {
	TheOLog << bege(lgAppState, lgcAll) << theStateCount << ende;
	Broadcast(TheLogStateChannel, &log);
}

int PolyApp::runWorkers(const int argc, char *argv[]) {
	try {
		WorkersRunner wr;
		wr.start(theWorkerCount, argc, argv);
		wr.monitor();
		return wr.sawError ? -1 : 0;
	}
	catch (const char *reason) {
		ShutdownReason(reason);
		return -1;
	}
}

void PolyApp::startServices() {
	TheStatPhaseMgr.start();
	TheStatCycle->start(); 
	if (theBeepDoorman)
		theBeepDoorman->start();
}

void PolyApp::startAgents() {
	Comment(1) << "starting " << theLocals.count() << " agents..." << endc;

	for (int i = 0; i < theLocals.count(); ++i)
		theLocals[i]->start();

	Comment(1) << "started " << theLocals.count() << " agents..." << endc;
}

int PolyApp::run(int argc, char *argv[]) {

	ThePrgName = argv[0];

	Clock::Update();

	if (!handleCmdLine(argc, argv))
		return -1;

	// must preceed configureRnd() call
	ThePersistWorkSetMgr.openInput(TheOpts.doLoadWorkSet);

	configureRnd(); // must preceed parsing
	parseConfigFile(TheOpts.theCfgFileName);

	Clock::Update();
	configure(); Clock::Update(false);
	reportCfg(); Clock::Update(false);
	logCfg(); Clock::Update(false);
	makeAgents(); Clock::Update(false);
	if (theWorkerCount) {
		ThePersistWorkSetMgr.close();
		return runWorkers(argc, argv);
	}

	loadPersistence();
	ThePersistWorkSetMgr.closeInput();

	logGlobals(); // after makeAgents (hence, POST/PUT ctypes) are computed
	describeLocals(); // after persistence (hence, agent IDs) is loaded

	flushState();

	// flush logs headers
	TheOLog.flush(); 
	TheSmplOLog->flush();

	// quit nicely on some signals
	// note: install this handler after configuration is done
	// so a program can be killed if configure takes forever
	// sent by shell on Ctrl-C
	signal(SIGINT, (SignalHandler*)&ShutdownSignal);
	// sent by kill and killall by default
	signal(SIGTERM, (SignalHandler*)&ShutdownSignal);

	// other signal handlers
	signal(SIGUSR1, (SignalHandler*)&DebugSignal);
#if FIND_MEMORY_LEAKS
	signal(SIGUSR2, (SignalHandler*)&DebugMemSignal);
#endif

	Clock::Update();

	startServices();

	Clock::Update();
	startAgents();

	Clock::Update();

	while (!ShutdownNow()) {
		step();
	}

	reportRUsage();
	flushState();

	ThePersistWorkSetMgr.openOutput(TheOpts.doStoreWorkSet);
	ThePersistWorkSetMgr.storeSeeds();
	storePersistence();
	ThePersistWorkSetMgr.close();

	return 0;
}


#if FIND_MEMORY_LEAKS
#warning FIND_MEMORY_LEAKS code is enabled, USR2 signal starts a sample
#include <stdlib.h>
#include <new>

int MemDebugIgnore = 0; // global
void *TheMainAddress = 0;
int MemDumpCount = 0;

static
void dumpStack(ostream &os) {
#ifdef __GNUC__
	void *addr = &dumpStack;
	// __builtin_return_address parameter must be a constant
#	define dumpOneAddress(level) \
		if (addr && addr > TheMainAddress) { \
			addr = __builtin_return_address(level); \
			cerr << '-' << addr; \
		}
	dumpOneAddress(1);
	dumpOneAddress(2);
	dumpOneAddress(3);
	dumpOneAddress(4);
	dumpOneAddress(5);
	dumpOneAddress(6);
	dumpOneAddress(7);
	dumpOneAddress(8);
	dumpOneAddress(9);
	dumpOneAddress(10);
	dumpOneAddress(11);
	dumpOneAddress(12);
	dumpOneAddress(13);
	dumpOneAddress(14);
	dumpOneAddress(15);
	dumpOneAddress(16);
	dumpOneAddress(17);
	dumpOneAddress(18);
	dumpOneAddress(19);
	dumpOneAddress(20);
	dumpOneAddress(21);
	dumpOneAddress(22);
	dumpOneAddress(23);
	dumpOneAddress(24);
	dumpOneAddress(25); // arbitrary limit
#endif
}

static
void countNew(size_t size, void *data, const char *kind) {
	static unsigned long calls = 0;
	calls++;
	if (MemDumpCount <= 0)
		return;

	/*if (size == 68 && *kind == 'i') // RndBodyIter
		return;
	if (size == 4 && *kind == 'i')
		return;
	if (size == 100 && *kind == 'a')
		return;
	if (size == 608 && *kind == 'i') // SrvXact
		return;
	*/

	if (MemDumpCount > 0) {
		if (size == 68 && *kind == 'i') {
			clog << here << "#new:  " << calls << " sz: " << size << " ptr: " << data << ' ' << kind <<
				" at " << MemDumpCount << " trace:";
			dumpStack(clog);
			clog << endl;
		}
		MemDumpCount--;
		//if (!MemDebugIgnore && size == 20 && *kind == 'a')
		//	abort();
	}
}

static
void countFree(void *data, const char *kind) {
	static unsigned long calls = 0;
	calls++;
	if (MemDumpCount <= 0)
		return;

	clog << here << "#free: " << calls << " sz: ? ptr: " << data << ' ' << kind << " at " << MemDumpCount << endl;
}

void *operator new(size_t size) throw (std::bad_alloc) {
	void *data = malloc(size);
	countNew(size, data, "item");
	return data;
}

void *operator new[](size_t size) throw (std::bad_alloc) {
	void *data = malloc(size);
	countNew(size, data, "arr");
	return data;
}

void operator delete(void *data) throw() {
	countFree(data, "item");
	free(data);
}

void operator delete[](void *data) throw() {
	countFree(data, "arr");
	free(data);
}

static
void DebugMemSignal(int s) {
	clog << "got debug signal (" << s << ')' << endl;
	MemDumpCount = 25000;
}

#endif


#if 0 /* main() template */
int main(int argc, char *argv[]) {
	// extern void *TheMainAddress;
	// TheMainAddress = &main;

	XxxApp app;
	return app.run(argc, argv);
}
#endif
