
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"
#include "xstd/h/process.h"  /* for _getpid() on W2K */

#include "xstd/h/math.h"
#include "xstd/h/os_std.h"
#include "xstd/h/string.h"
#include "xstd/h/signal.h"
#include "xstd/h/netinet.h"

#include <fstream>

#include "xstd/Assert.h"
#include "xstd/Time.h"
#include "xstd/Socket.h"
#include "xstd/Select.h"
#include "xstd/Poll.h"
#include "xstd/Epoll.h"
#include "xstd/AlarmClock.h"
#include "xstd/History.h"
#include "base/AggrStat.h"
#include "base/polyLogCats.h"
#include "base/polyVersion.h"
#include "runtime/NotifMsg.h"


// handles incoming messages
class MsgMonitor: public FileScanUser {
	public:
		MsgMonitor(Socket aSock);
		virtual ~MsgMonitor();

		virtual void noteReadReady(int fd);

	public:
		Socket theSock;
		FileScanReserv theReserv;

		char theBuf[sizeof(StatusFwdMsg)*64];
		int theSize;
};

// handles periodic operations
class Ticker: public AlarmUser {
	public:
		Ticker(Time aPeriod);
		virtual ~Ticker();

		virtual void wakeUp(const Alarm &alarm);

	public:
		Time thePeriod;
};

struct HostFilter;

// maintains info about a host
class Host {
	public:
		typedef History<StatusFwdMsg> Log;

	public:
		Host(const NetAddr &anAddr);

		const NetAddr &addr() const { return theAddr; }
		// XXX pre-IPv6 code used 's_addr' as an int
		int id() const { return theAddr.addrN().octet(0); }
		int logCat() const;
		// XXX pre-IPv6 code used 's_addr' as an int
		int lna() const { return theAddr.addrN().octet(0); }
		const Log &log() const { return theLog; }
		const char *runLabel() const;

		bool busy() const;
		bool selected() const { return isSelected; }
		bool isClient() const { return logCat() == lgcCltSide; }
		bool isServer() const { return logCat() == lgcSrvSide; }
	
		void noteMsg(const StatusFwdMsg &msg); // { theLog.insert(msg); }
		void selected(bool be) { isSelected = be; }

		bool matches(const HostFilter &sel) const;

	protected:
		NetAddr theAddr; // unique host address
		Log theLog;      // notification messasge history
		bool isSelected;
};

// used to select groups of hosts
struct HostFilter {
	struct {
		int pos;
	} lbl;          // experiment label based selection
	struct {
		int pos;
	} logCat;           // category based selection (aka "side")

	HostFilter() { memset(this, 0, sizeof(*this)); }
};

class MsgSum {
	public:
		MsgSum();

		Counter hostCount() const { return theReqRate.count(); }

		MsgSum &operator +=(const StatusFwdMsg &msg);

	public:
		String theLabels;
		AggrStat theReqRate;
		AggrStat theRepRate;
		AggrStat theBwidth;
		AggrStat theRespTime;
		AggrStat theDHR;
		AggrStat theConnUse;
		AggrStat theErrRatio;
		AggrStat theErrTotCnt;
		AggrStat theXactTotCnt;
		AggrStat theTotErrRatio;
		AggrStat theSockInstCnt;
};


/* globals */

static NetAddr TheDisp;

static FileScanner *TheScanner = 0;
static bool DoShutdown = false;

static Array<Host*> TheHosts;
static Array<Host*> TheHostIdx; // address -> host map
static int TheBusyHostCnt = 0;
static Array<String*> TheLabels;
static int TheUniqLblCnt = 0;

static void NoteMsg(const StatusFwdMsg &msg);
static void DeleteIdleHosts();
static void RrdUpdate();
static bool AddFirstLabel(const Host *skip, const String &l);
static bool DelLastLabel(const Host *skip, const String &l);

static String DbaseName = "polygraph.rrd";

inline int Dbl2Int(double v) { return (int)rint(v); }


/* MsgMonitor */

MsgMonitor::MsgMonitor(Socket aSock): theSock(aSock), theSize(0) {
	theReserv = TheScanner->setFD(theSock.fd(), dirRead, this);
}

MsgMonitor::~MsgMonitor() {
	if (theReserv)
		TheScanner->clearRes(theReserv);
	if (theSock)
		theSock.close();
}

void MsgMonitor::noteReadReady(int) {
	static const int msgsz = sizeof(StatusFwdMsg);

	const int sz = theSock.read(theBuf + theSize, sizeof(theBuf) - theSize);

	if (sz < 0) {
		if (Error::Last() != EWOULDBLOCK) {
			cerr << "failed to read from dispatcher at " << TheDisp 
				<< ": " << Error::Last() << endl;
			DoShutdown = true;
		}
		return;
	}
 
	if (sz == 0) {
		cerr << "dispatcher at " << TheDisp << " quit." << endl;
		DoShutdown = true;
		return;
	}

	theSize += sz;

	while (theSize >= msgsz) {
		StatusFwdMsg msg;
		memcpy(&msg, theBuf, msgsz);
		theSize -= msgsz;
		memmove(theBuf, theBuf + msgsz, theSize);

		// handle the message
		msg.ntoh();
		msg.theRcvTime = TheClock;
		NoteMsg(msg);
	}
	Assert(theSize >= 0);
}


/* Ticker */

Ticker::Ticker(Time aPeriod): thePeriod(aPeriod) {
	sleepFor(thePeriod);
}

Ticker::~Ticker() {
}

void Ticker::wakeUp(const Alarm &alarm) {
	AlarmUser::wakeUp(alarm);

	DeleteIdleHosts();
	RrdUpdate();

	sleepFor(thePeriod);
}


/* Host */

Host::Host(const NetAddr &anAddr): theAddr(anAddr), theLog(60), isSelected(false) {
}

int Host::logCat() const {
	return theLog.depth() ? theLog[0].theCat : lgcAll;
}

const char *Host::runLabel() const {
	return theLog.depth() ? theLog[0].theLabel : 0;
}

bool Host::busy() const {
	return theLog.depth() && theLog[0].theRcvTime > TheClock.time() - Time::Sec(90);
}

void Host::noteMsg(const StatusFwdMsg &msg) {
	theLog.insert(msg);
}

bool Host::matches(const HostFilter &filter) const {
	if (filter.logCat.pos != lgcAll && filter.logCat.pos != logCat())
		return false;
	if (const int l = filter.lbl.pos)
		return theLog.depth() && *TheLabels[l] == theLog[0].theLabel;
	return true;
}


/* MsgSum */

MsgSum::MsgSum(): theLabels("") {
}

MsgSum &MsgSum::operator +=(const StatusFwdMsg &msg) {
	// XXX: theLabels is not updated
	theReqRate.record(Dbl2Int(msg.theReqRate));
	theRepRate.record(Dbl2Int(msg.theRepRate));
	theBwidth.record(Dbl2Int(msg.theBwidth));
	theRespTime.record(msg.theRespTime.msec());
	theDHR.record(Dbl2Int(msg.theDHR*100));
	theConnUse.record(Dbl2Int(msg.theConnUse));
	theErrRatio.record(Dbl2Int(msg.theErrRatio*100));
	theErrTotCnt.record(msg.theErrTotCnt);
	theXactTotCnt.record(msg.theXactTotCnt);
	theTotErrRatio.record(
		Dbl2Int(Percent(msg.theErrTotCnt, msg.theErrTotCnt+msg.theXactTotCnt)));
	theSockInstCnt.record(msg.theSockInstCnt);
	return *this;
}

/* local routines */

static
Host *AddHost(const NetAddr &addr) {
	Host *host = new Host(addr);
	Host *foundPos = 0;
	for (int i = 0; !foundPos && i < TheHosts.count(); ++i) {
		if (!TheHosts[i])
			foundPos = TheHosts[i] = host;
	}
	if (!foundPos)
		TheHosts.append(host);
	TheBusyHostCnt++;
	return host;
}

static
void NoteMsg(const StatusFwdMsg &msg) {
	//clog << "from " << inet_ntoa(from.sin_addr) << ": " << TheMsg.buf << endl;

	NetAddr from(msg.theSndAddr.addr, msg.theSndAddr.port);

	// find corresponding host
	// XXX pre-IPv6 code used 's_addr' as an int
	const int hidx = from.addrN().octet(0) % TheHostIdx.count();
	Host *host = TheHostIdx[hidx];

	if (host && host->addr() != from) { // collision
		host = 0;
		// find using linear search
		for (int i = 0; !host && i < TheHosts.count(); ++i) {
			if (TheHosts[i] && TheHosts[i]->addr() == from)
				host = TheHosts[i];
		}
	}

	if (!host) {
		host = AddHost(from);
		if (!TheHostIdx[hidx])
			TheHostIdx[hidx] = host;
		AddFirstLabel(host, msg.theLabel);
	}

	host->noteMsg(msg);
}


static
void DeleteIdleHosts() {
	// we will build these from scratch:
	TheHostIdx.clear();

	for (int h = 0; h < TheHosts.count(); ++h) {
		if (Host *host = TheHosts[h]) {
			if (host->busy()) {
				// XXX pre-IPv6 code used 's_addr' as an int
				const int idx = host->addr().addrN().octet(0) % TheHostIdx.count();
				TheHostIdx[idx] = host;
			} else {
				TheBusyHostCnt--;
				DelLastLabel(host, host->runLabel());
				delete host;
				TheHosts[h] = 0;
			}
		}
	}
}

static
void RrdUpdate() {
	const Time freshCutOff = TheClock - Time::Sec(60);
	MsgSum snapshot; // last values available
	MsgSum window;   // all fresh values in the logs
	for (int h = 0; h < TheHosts.count(); ++h) {
		Host *host = TheHosts[h];
		if (!host)
			continue;
		if (host->logCat() != lgcCltSide)
			continue;

		// should be subject to freshness rule as well?
		snapshot += host->log()[0];

		for (int l = 0; l < host->log().depth(); ++l) {
			const StatusFwdMsg &msg = host->log()[l];
			if (freshCutOff <= msg.theRcvTime)
				window += msg;
		}
	}

	if (snapshot.hostCount()) {
		ostringstream cmdbuf;
		configureStream(cmdbuf, 2);
		cmdbuf
			<< "rrdtool update " << DbaseName
			<< " --template xact:err:rt:sock:dhr N"
			<< ':' << snapshot.theXactTotCnt.sum()
			<< ':' << snapshot.theErrTotCnt.sum()
			<< ':' << window.theRespTime.mean()
			<< ':' << snapshot.theSockInstCnt.sum()
			<< ':' << window.theDHR.mean()
			<< ends;

		const String cmd = cmdbuf.str().c_str();
		streamFreeze(cmdbuf, false);

		cout << cmd << endl;
		Should(system(cmd.cstr()) == 0);
	}
}

static
bool AddFirstLabel(const Host *skip, const String &l) {
	for (int h = 0; h < TheHosts.count(); ++h) {
		if (TheHosts[h] == skip)
			continue;
		if (TheHosts[h] && l == TheHosts[h]->runLabel())
			return false; // not first label
	}
	TheUniqLblCnt++;
	for (int i = 1; i < TheLabels.count(); ++i) {
		if (!*TheLabels[i]) {
			*TheLabels[i] = l;
			return true;
		}
	}
	TheLabels.append(new String(l));
	return false;
}

static
bool DelLastLabel(const Host *skip, const String &l) {
	for (int h = 0; h < TheHosts.count(); ++h) {
		if (TheHosts[h] == skip)
			continue;
		if (TheHosts[h] && l == TheHosts[h]->runLabel())
			return false; // not last label
	}
	TheUniqLblCnt--;
	for (int i = 1; i < TheLabels.count(); ++i) {
		if (*TheLabels[i] == l) {
			*TheLabels[i] = 0;
			if (i == TheLabels.count()-1)
				delete TheLabels.pop();
			return true;
		}
	}
	Assert(0);
	return false;
}

int main(int argc, char *argv[]) {
	(void)PolyVersion();

	int argOff = 0;
	if (argc > 1 && String(argv[1]) == "--help") {
		cout << "Usage: " << argv[0]
			<< " [--help] [--database <filename>]"
			<< " [udp2tcpd_ip [udp2tcpd_port]]"
			<< endl;
		return 0;
	} else
	if (argc > 1 && String(argv[1]) == "--database") {
		DbaseName = argv[2];
		argOff += 2;
		argc -= 2;
	}

	const char *disph = argc >= 2 ? argv[argOff+1] : "127.0.0.1";
	const int dispp = argc >= 3 ? atoi(argv[argOff+2]) : 18256;
	TheDisp = NetAddr(disph, dispp);

	Socket sock;
	Must(sock.create(TheDisp.addrN().family()));
	if (!sock.connect(TheDisp)) {
		cerr << "failed to connect to udp2tcp dispatcher at " << TheDisp
			<< ": " << Error::Last() << endl;
		return -2;
	}
	Should(sock.blocking(false));

	TheHostIdx.resize(256);
	TheLabels.append(new String);

	signal(SIGPIPE, SIG_IGN);
	Clock::Update();

	TheScanner = new PG_PREFFERED_FILE_SCANNER;
	TheScanner->configure();

	MsgMonitor *msgSrv = new MsgMonitor(sock);
	Ticker *ticker = new Ticker(Time::Sec(60));

	const String cmd = String("test -f ") + DbaseName +
		" || rrdtool create " + DbaseName +
		" --step 60 "
		"DS:xact:DERIVE:90:0:U "
		"DS:err:DERIVE:90:0:U "
		"DS:rt:GAUGE:90:0:10000 "	// require 0-10,000 msec
		"DS:sock:GAUGE:90:0:U "
		"DS:dhr:GAUGE:90:0:100 "
		"RRA:AVERAGE:0.50:1:1440 "	// 60 mins per hour
		"RRA:AVERAGE:0.50:5:864 "	// 864 5mins per 3days
		"RRA:AVERAGE:0.50:60:168"	// 168 hours per week
		;

	clog << "executing: " << cmd << endl;
	Must(system(cmd.cstr()) == 0);

	// store our pid
	const String pidFname = "polyrrd.pid";
	{
		ofstream pidFile(pidFname.cstr());
		pidFile << getpid() << endl;
	}

	while (!DoShutdown) {
		Clock::Update();
		Time tout = TheAlarmClock.timeLeft();
		TheScanner->scan(TheAlarmClock.on() ? 0 : &tout);
	}

	delete ticker;
	delete msgSrv;
	delete TheScanner;

	unlink(pidFname.cstr());

	return 0;
}
