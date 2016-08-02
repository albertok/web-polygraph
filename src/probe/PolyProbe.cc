
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>

#include "xstd/h/signal.h"
#include "xstd/h/sstream.h"
#include "xstd/h/net/if.h"
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Assert.h"
#include "xstd/FileScanner.h"
#include "xstd/Error.h"
#include "xstd/getIfAddrs.h"
#include "xstd/gadgets.h"
#include "xstd/InetIfReq.h"
#include "base/CmdLine.h"
#include "base/opts.h"
#include "base/polyLogTags.h"
#include "pgl/PglNetAddrRange.h"
#include "runtime/BcastSender.h"
#include "runtime/polyBcastChannels.h"
#include "probe/ProbeClt.h"
#include "probe/ProbeOpts.h"
#include "probe/ProbeStatMgr.h"
#include "probe/PolyProbe.h"

PolyProbe *ThePolyProbe = 0;
FileScanner *TheFileScanner = 0;

static Array<InAddress> TheLocalHosts;

// XXX: Here to avoid linking with runtime/libagent.a and
// opening the associated Pandora's box filled with linking problems.
void StopTrafficWaiting() {}

// XXX: should not be needed! remove magic labels/versions from OLog
class MyOLog: public OLog {
	public:
		MyOLog() {}

	protected:
		virtual void putHeader();
};

void MyOLog::putHeader() {
	puti(lgMagic1); puti(lgMagic2); puti(0); // magic
	OLog::putHeader();
}


PolyProbe::PolyProbe(): exchangingStats(false), mustStop(false) {
	Assert(!ThePolyProbe);
	ThePolyProbe = this;
}

PolyProbe::~PolyProbe() {
	while (theAgents.count()) delete theAgents.pop();
}

bool PolyProbe::handleCmdLine(int argc, char *argv[]) {

	Array<OptGrp*> opts;
	opts.append(&TheProbeOpts);

	theCmdLine.configure(opts);
	if (!theCmdLine.parse(argc, argv))
		return false;

	// validate command line params
	for (int i = 0; i < opts.count(); ++i)
		if (!opts[i]->validate())
			return false;

	return true;
}

void PolyProbe::configureLogs(int prec) {
	// redirect console output
	if (TheProbeOpts.theConsFileName && TheProbeOpts.theConsFileName != "-")
		redirectOutput(TheProbeOpts.theConsFileName.cstr());

	configureStream(cout, prec);
	configureStream(cerr, prec);
	configureStream(clog, prec);
}

void PolyProbe::configureHosts() {
	Array<NetAddr*> lclClients;
	configureHosts(TheProbeOpts.theCltHosts.val(), theAllClients, lclClients);

	Array<NetAddr*> lclServers;
	configureHosts(TheProbeOpts.theSrvHosts.val(), theAllServers, lclServers);

	if (!lclClients.count() && !lclServers.count()) {
		cerr << thePrgName << ": no specified host addresses match local addresses" << endl;
		dumpHostSpace(lclClients, lclServers);
		exit(-3);
	}

	// all servers must have ports
	for (int i = 0; i < theAllServers.count(); ++i) {
		if (theAllServers[i]->port() <= 0)
			cerr << *theAllServers[i] << ": server address lacks port number" 
				<< endl << xexit;
	}

	// add servers first so that they are up and running before local clients
	for (int s = 0; s < lclServers.count(); ++s) {
		theServers.append(new ProbeSrv(*lclServers[s]));
		theAgents.append(theServers.last());
	}

	for (int c = 0; c < lclClients.count(); ++c) {
		for (int s = 0; s < theAllServers.count(); ++s) {
			theClients.append(new ProbeClt(*lclClients[c], *theAllServers[s]));
			theAgents.append(theClients.last());
		}
	}

	if (lclServers.count())
		TheProbeStatMgr.incConfigure(theAllClients, lclServers);

	if (lclClients.count())
		TheProbeStatMgr.incConfigure(lclClients, theAllServers);
}

void PolyProbe::configure() {
	configureLogs(2);

	// ignore some signals
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	setResourceLimits();
	configureHosts();
}

void PolyProbe::start() {
	if (TheProbeOpts.theDuration >= 0)
		sleepFor(TheProbeOpts.theDuration);

	for (int a = 0; a < theAgents.count(); ++a)
		theAgents[a]->exec();
}

// XXX: merge with common stuff in PolyApp::step()
void PolyProbe::step(Time tout) {
	Clock::Update(false);

	Time *toutp = &tout;

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

	const int readyCount = TheFileScanner->scan(toutp);

	if (mustStop)
		return;

	if (readyCount < 0) {
		if (const Error err = Error::LastExcept(EINTR))
			cerr << thePrgName << ": fatal error: " << err << endl << xexit;
	}

	Clock::Update();
}

void PolyProbe::wakeUp(const Alarm &a) {
	AlarmUser::wakeUp(a);
	if (!exchangingStats) {
		if (theAllClients.count()) {
			pullStats();
			sleepFor(TheProbeOpts.theStatExchTout);
		} else {
			// servers do not pull stats
			exchangingStats = true;
			mustStop = true;
		}
	} else {
		// exchange stats timeout
		clog << "stats exchange timeout" << endl;
		mustStop = true;
	}
}

void PolyProbe::setResourceLimits() {
	TheFileScanner = TheProbeOpts.theFileScanner.val();
	TheFileScanner->configure();
	TheFileScanner->ticker(0);

	// to be safe, cut about 3% from the actual limit
	const int fdLimit = TheFileScanner->fdLimit();
	Socket::TheMaxLevel = Max(0, fdLimit - 10 - fdLimit/33);
}

void PolyProbe::configureHosts(const Array<String*> &ranges, Array<NetAddr*> &allHosts, Array<NetAddr*> &lclHosts) {
	// put command line addresses to allHosts array
	for (int i = 0; i < ranges.count(); ++i) {
		const String &host = *ranges[i];
		PglNetAddrRange hostParser;
		if (!hostParser.parse(host))
			cerr << thePrgName << ": malformed host address: `" << host << endl << xexit;
		hostParser.toAddrs(allHosts);
	}

	// find all local addresses (once)
	Array<InetIfReq> ifaces;
	if (!TheLocalHosts.count() && !GetIfAddrs(ifaces, String()))
		cerr << thePrgName << ": cannot get a list of all available addresses: " << Error::Last() << endl << xexit;

	TheLocalHosts.stretch(ifaces.count());
	for (int l = 0; l < ifaces.count(); ++l)
		TheLocalHosts.append(ifaces[l].addrN());

	// match local and configured addresses
	Array<int> matches;
	for (int h = 0; h < allHosts.count(); ++h) {
		const NetAddr &host = *allHosts[h];

		// check if the host address is local
		bool found = false;
		for (int l = 0; !found && l < TheLocalHosts.count(); ++l)
			found = TheLocalHosts[l] == host.addrN();

		if (found)
			matches.append(h);
	}

	for (int m = 0; m < matches.count(); ++m)
		lclHosts.append(allHosts[matches[m]]);
}

void PolyProbe::dumpHostSpace(const Array<NetAddr*> &clients, const Array<NetAddr*> &servers) {
	TheProbeOpts.theCltHosts.report(cerr << "all client host addresses:");
	cerr << endl;
	dumpHosts(cerr << "local client addresses: ", clients);
	cerr << endl;

	cerr << endl;

	TheProbeOpts.theSrvHosts.report(cerr << "all server host addresses:");
	cerr << endl;
	dumpHosts(cerr << "local server addresses: ", servers);
	cerr << endl;

	cerr << endl;

	dumpHosts(cerr << "all local addresses: ", TheLocalHosts);
	cerr << endl;
}

void PolyProbe::dumpHosts(ostream &os, const Array<InAddress> &hosts) {
	for (int h = 0; h < hosts.count(); ++h)
		os << ' ' << hosts[h].image();
}

void PolyProbe::dumpHosts(ostream &os, const Array<NetAddr*> &hosts) {
	for (int h = 0; h < hosts.count(); ++h)
		os << ' ' << *hosts[h];
}

void PolyProbe::reportCfg() const {
	theCmdLine.reportRaw(clog << thePrgName << ": "); clog << endl;
	theCmdLine.reportParsed(clog << thePrgName << ": "); clog << endl;
}

// called when somebody wants our stats
void PolyProbe::sendStats(Socket &s, const NetAddr &to) {
	Assert(s.fd() >= 0);

	cerr << here << "sending stats to " << to << endl;

	char buf[5*1024*1024];
	ofixedstream *os = new ofixedstream(buf, sizeof(buf));
	MyOLog log;
	log.stream(to.addrA(), os);
	TheProbeStatMgr.exportStats(log);
	log.flush();
	const Size sz = Size(os->tellp()); // XXX: wrong, close() adds more!
	log.close();

	s.blocking(true);
	Should(s.write(buf, sz) == sz && sz < SizeOf(buf));
	s.close();
}

void PolyProbe::pullStats() {
	exchangingStats = true;
	// XXX: should broadcast only once
	Broadcast(ThePhasesEndChannel, BcastRcver::ieNone);

	// pull stats from for all servers
	for (int i = 0; i < theAllServers.count(); step(Time(0,0)), ++i) {
		NetAddr &rhost = *theAllServers[i];

		// blocking ops OK here? no deadlock danger?
		Socket s;
		Must(s.create(rhost.addrN().family()));
		Must(s.blocking(true));
		Must(s.connect(rhost));

		// ask for stats
		{
			const int cmd = htonl(0xFFFFFFFF);
			const Size sz = s.write(&cmd, sizeof(cmd));
			if (!Should(sz == sizeof(cmd)))
				continue;
		}
		
		// read stats
		{
			char buf[5*1024*1024];
			bool err = false;
			Size pos = 0;
			while (!err && pos < SizeOf(buf)) {
				const Size sz = s.read(buf + pos, SizeOf(buf) - pos);
				err = !Should(sz >= 0);
				pos += sz;
				if (!sz)
					break;
			}
			err = err || !Should(pos < SizeOf(buf));
			if (err)
				continue;

			istringstream is(string(buf, pos));
			ILog log;
			log.stream(rhost.addrA(), &is);
			TheProbeStatMgr.importStats(log);
		}
	}
	mustStop = true;
}

void PolyProbe::reportStats() const {
	TheProbeStatMgr.report(cout);
}

int PolyProbe::run(int argc, char *argv[]) {

	thePrgName = argv[0];

	if (!handleCmdLine(argc, argv))
		return -1;

	Clock::Update();

	configure();

	reportCfg();

	start();

	while (!mustStop) {
		step(Time());
	}

	reportStats();

	return 0;
}


int main(int argc, char *argv[]) {
	PolyProbe app;
	return app.run(argc, argv);
}
