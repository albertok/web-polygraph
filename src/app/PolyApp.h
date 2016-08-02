
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__APP_POLYAPP_H
#define POLYGRAPH__APP_POLYAPP_H

#include <map>
#include <set>
#include "xstd/NetAddr.h"
#include "xstd/String.h"
#include "xstd/Array.h"
#include "xstd/FileScanner.h"
#include "base/CmdLine.h"
#include "runtime/BcastRcver.h"

class OLog;
class Agent;
class AgentSym;
class ContentSel;
class HostMap;
class SizeOpt;
class NetAddrSym;
class BeepDoorman;
class RobotSym;
class HostCfg;

// common interface for polyclt and polysrv applications
class PolyApp: public FileScanTicker, public BcastRcver {
	public:
		typedef Array<NetAddrSym*> AddrSyms;

	public:
		PolyApp();
		virtual ~PolyApp();

		virtual void noteMsgStrEvent(BcastChannel *ch, const char *msg);
		virtual void begCycle(int readyCount);
		virtual void endCycle();
		virtual bool tick();


		int run(int argc, char *argv[]);
		void flushState();

	protected:
		typedef std::map<String, int> AliasIndexes; // map iface : next free alias index

	protected:
		bool handleCmdLine(int argc, char *argv[]);
		void makeAgents();
		void checkProgressReport() const;
		void reportRUsage() const;

		virtual void configure();
		virtual void reportCfg();
		virtual void logCfg();
		virtual void logGlobals();
		virtual void step();

		// except for random seeds that are handled earlier
		virtual void loadPersistence();
		virtual void storePersistence();
		
		void configureBinLog(OLog &log, const String &fname, SizeOpt &bufSz);
		void configureLogs(int prec);
		void configureRnd();
		void configureHosts();
		void configureRobotContent(const RobotSym &clt, const String &fieldName);
		void buildSchedule();
		void parseConfigFile(const String &fname);

		int scan(Time *toutp);
		
		void checkTiming(Time drift);

		void getIfaces();
		void getFakeIfaces();
		void getHostAddrs(Array<NetAddr*> &hosts) const;
		void getCpuCores(Array< Array<int> > &cpuCores) const;
		void makeAddresses();
		ostream &makeAddresses(Array<NetAddr*> &hosts, AddrSyms &agents, ostream &err);
		int makeAddresses(int hidx, AddrSyms &agents, int agentsPerHost, AliasIndexes& aliasIndexes);
		void deleteAddresses(const String &ifname);
		void addUniqueAddrs(AddrSyms *const addrs, const NetAddrSym &s, HostMap &seen, Array<NetAddr> &skipped, HostMap &skipped_map) const;

		void getAgentAliasAddrs(AddrSyms &agents) const;
		void addAgent(Agent *agent);
		void describeLocals() const;
		int runWorkers(int argc, char *argv[]);
		HostCfg *addToHostMap(const NetAddr &host, int &idx);
		
		virtual void startServices();
		virtual void startAgents();
		virtual void getOpts(Array<OptGrp*> &opts);
		virtual void wasIdle(Time tout);
		virtual Agent *makeAgent(const AgentSym &agent, const NetAddr &address) = 0;
		virtual void logState(OLog &log);

		virtual const String sideName() const = 0;
		virtual int logCat() const = 0;

	protected:
		CmdLine theCmdLine;

		String thePglCfg;            // temporary cache for logging purposes
		Array<InAddress> theIfaces;    // list of interface or fake addresses
		Array<Agent*> theLocals;
		Array<ContentSel*> theContentSels;
		String theAgentType;

		BeepDoorman *theBeepDoorman;

		Time theIdleBeg;  // the start of an idle period
		Time theIdleEnd;  // beg + idle_tout
		bool isIdle;

		int theTickCount;  // see PolyApp::tick()
		int theStateCount; // number of states [flushed]
		int theWorkerCount; // number of child processes
};

#endif
