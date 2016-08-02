
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"
#include "base/polyVersion.h"

// do not try to compile polymon if curses are not supported
#if defined(HAVE_CONFIG_H) && (!defined(HAVE_NCURSES_H) || !defined(HAVE_LIBNCURSES))
	int main() { 
		cerr << "polymon could not be built without "
			<< "a properly installed ncurses library" << endl; 
		return -1; 
	}
#else

#include "xstd/h/math.h"
#include "xstd/h/os_std.h"
#include "xstd/h/string.h"
#include "xstd/h/signal.h"
#include "xstd/h/netinet.h"

#include <stdlib.h>

#if defined(sun)
#define bool BOOL
#endif
#include <ncurses.h>
// cleanup after ncurses.h
#if defined(bool)
#undef bool
#endif
#if defined(clear)
#undef clear
#endif
#if defined(timeout)
#undef timeout
#endif

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
#include "runtime/NotifMsg.h"



// takes care of the keyboard input
class KbdMonitor: public FileScanUser {
	public:
		KbdMonitor(int aFD);
		virtual ~KbdMonitor();

		virtual void noteReadReady(int fd);

	public:
		int theFD;
		FileScanReserv theReserv;
};

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

class HostFilter;

// maintains info about a host
class Host {
	public:
		typedef History<StatusFwdMsg> Log;

	public:
		Host(const NetAddr &anAddr);

		const NetAddr &addr() const { return theAddr; }
		// XXX pre-IPv6 code returned 's_addr' as an int
		int id() const { return theAddr.addrN().octet(0); }
		int logCat() const;
		// XXX pre-IPv6 code returned 's_addr' as an int
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


// used in some "matrix" windows to remember screen content
class WinMatrix {
	public:
		WinMatrix(int aMaxY, int aMaxX);

		double operator ()(int y, int x) const { return theImage[safePos(y,x)]; }
		double &operator ()(int y, int x) { return theImage[safePos(y,x)]; }

	protected:
		int safePos(int y, int x) const;

	protected:
		Array<double> theImage;
		int theMaxY;
		int theMaxX;
};

// WINDOW wrapper with a title and event handlers
class Window {
	public:
		typedef void (Window::*EventHandler)(const Host &);

	public:
		Window(const char *aTitle);
		virtual ~Window();

		virtual void noteAdd(const Host &) {}
		virtual void noteDel(const Host &) {}
		virtual void noteUpd(const Host &) {}

	private:
		// disable copying
		Window(const Window &);
		Window &operator =(const Window &);

	public:
		WINDOW *const handle;
		const char *title;
};

// general monitor information
class InfoWindow: public Window {
	public:
		InfoWindow(const char *aTitle);

		virtual void noteAdd(const Host &);
		virtual void noteDel(const Host &);
		virtual void noteUpd(const Host &);

	protected:
		void update();

	protected:
		int theMsgCnt;       // msgs seen so far
		int theListnCnt;     // derived from CopyCnt in the fwded msg
};

// help window
class HelpWindow: public Window {
	public:
		HelpWindow();

	protected:
		void update();
};

// summary information about SmxWindows
class MsgSum;
class SumWindow: public Window {
	public:
		SumWindow(const char *aTitle);

		virtual void noteAdd(const Host &);
		virtual void noteDel(const Host &);
		virtual void noteUpd(const Host &);

	protected:
		void update();
		void displaySide(int &y, int x, const MsgSum &sum);
		void displayLine(int y, int x, const char *label, const AggrStat &stats, const char *meas, double scale = 1);
};

// single measurement matrix window
class SmxWindow: public Window {
	public:
		SmxWindow(const char *aTitle);

		virtual void noteAdd(const Host &);
		virtual void noteDel(const Host &);
		virtual void noteUpd(const Host &);

	protected:
		virtual void displayMsg(int y, int x, const StatusFwdMsg &msg) = 0;
		virtual void eraseSlot(int y, int x);

	protected:
		static int Host2X(const Host &host);
		static int Host2Y(const Host &host);
		static int Id2X(int id);
		static int Id2Y(int id);
};

// number matrix window
class NumxWindow: public SmxWindow {
	public:
		NumxWindow(const char *aTitle, const char *aFmt);

		virtual void noteAdd(const Host &);
		virtual void noteDel(const Host &);
		virtual void noteUpd(const Host &);

	protected:
		virtual void displayMsg(int y, int x, const StatusFwdMsg &msg);		
		virtual void eraseSlot(int y, int x);

		virtual double msg2num(const StatusFwdMsg &msg) = 0;

		void update();

	protected:
		const char *theFmt;  // format to use for output
		WinMatrix theMatrix; // currently displayed values
		double theNumSum;    // accumulator to compute averages and sums
		int theNumCnt;       // number of hosts contributed to sum
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
		AggrStat theXactTotCnt;
		AggrStat theTotErrRatio;
		AggrStat theSockInstCnt;
};

/* smx windows for all attributes */

struct RunLabelSmxWin: public SmxWindow {
	RunLabelSmxWin(const char *aTitle): SmxWindow(aTitle) {}
	virtual void displayMsg(int y, int x, const StatusFwdMsg &msg) {
		mvwprintw(handle, y, x, (char*)"%7.6s", msg.theLabel);
	}
};

struct RunTimeSmxWin: public SmxWindow {
	RunTimeSmxWin(const char *aTitle): SmxWindow(aTitle) {}
	virtual void displayMsg(int y, int x, const StatusFwdMsg &msg);
};

struct ReqRateSmxWin: public NumxWindow {
	ReqRateSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.1f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return (double)msg.theReqRate;
	}
};

struct RepRateSmxWin: public NumxWindow {
	RepRateSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.1f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return (double)msg.theRepRate;
	}
};

struct BwidthSmxWin: public NumxWindow {
	BwidthSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.1f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return (double)msg.theBwidth/(1024.*1024/8);
	}
};

struct RespTimeSmxWin: public NumxWindow {
	RespTimeSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.0f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return msg.theRespTime.msec();
	}
};

struct DHRSmxWin: public NumxWindow {
	DHRSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.2f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return Max(100*(double)msg.theDHR, -1.);
	}
};

struct ConnUseSmxWin: public NumxWindow {
	ConnUseSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.2f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return msg.theConnUse;
	}
};

struct ErrRatioSmxWin: public NumxWindow {
	ErrRatioSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.2f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return Max(100*(double)msg.theErrRatio, -1.);
	}
};

struct XactTotCntSmxWin: public NumxWindow {
	XactTotCntSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.0f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return msg.theXactTotCnt/1000.;
	}
};

struct ErrTotCntSmxWin: public NumxWindow {
	ErrTotCntSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.2f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return msg.theErrTotCnt/1000.;
	}
};

struct ErrTotRatioSmxWin: public NumxWindow {
	ErrTotRatioSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.2f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return Percent(msg.theErrTotCnt, msg.theXactTotCnt);
	}
};

struct SockInstCntSmxWin: public NumxWindow {
	SockInstCntSmxWin(const char *aTitle): NumxWindow(aTitle, "%7.0f") {}
	virtual double msg2num(const StatusFwdMsg &msg) {
		return msg.theSockInstCnt;
	}
};

class MsgGapSmxWin: public SmxWindow {
	public:
		MsgGapSmxWin(const char *aTitle): SmxWindow(aTitle) {}

		virtual void noteUpd(const Host &host);

	protected:
		virtual void displayMsg(int y, int x, const StatusFwdMsg &msg);

	protected:
		StatusFwdMsg theLastMsg;
};


/* globals */

static NetAddr TheDisp;

static FileScanner *TheScanner = 0;
static bool DoShutdown = false;

static PtrArray<Host*> TheHosts;
static Array<Host*> TheHostIdx; // address -> host map
static int TheBusyHostCnt = 0;
static PtrArray<String*> TheLabels;
static int TheUniqLblCnt = 0;

static PtrArray<Window*> TheWins;
static int TheWinPos = 0; // current window
static Window *TheInfoWin = 0; // special

static Window *TheHelpWin = 0; // help window
static bool ShowingHelp = false; // true iff help window is shown

static HostFilter TheFilter;
static char TheFiltWarn[80] = "";
static int TheSelHostCnt = 0;

static const int TheXMargin = 5;
static const int TheYMargin = 4;
static const int TheXCellWidth = 7;
static const int TheYCellWidth = 2;

static void NoteMsg(const StatusFwdMsg &msg);
static void Broadcast(Window::EventHandler weh, const Host &host);
static void DeleteIdleHosts();
static bool AddFirstLabel(const Host *skip, const String &l);
static bool DelLastLabel(const Host *skip, const String &l);
static void SelectHost(Host *host);
static int SwitchWin(int idx);
static int SwitchLblFilter(int idx);
static int SwitchCatFilter(int idx);
static void BuildFiltWarn(ostream &os);

inline int Dbl2Int(double v) { return (int)rint(v); }


/* KbdMonitor */

KbdMonitor::KbdMonitor(int aFD): theFD(aFD) {
	theReserv = TheScanner->setFD(theFD, dirRead, this);
}

KbdMonitor::~KbdMonitor() {
	if (theReserv)
		TheScanner->clearRes(theReserv);
}

void KbdMonitor::noteReadReady(int) {

	switch (getch()) {
		case '0': SwitchWin(0); break;

		case KEY_LEFT: SwitchWin(TheWinPos-1); break;
		case KEY_RIGHT: SwitchWin(TheWinPos+1); break;

		case KEY_DOWN: SwitchLblFilter(TheFilter.lbl.pos - 1); break;
		case KEY_UP: SwitchLblFilter(TheFilter.lbl.pos + 1); break;

		case 'H':
		case 'h':
			if (!ShowingHelp) {
				ShowingHelp = true;
				touchwin(TheHelpWin->handle);
				wrefresh(TheHelpWin->handle);
				return;
			} else
				SwitchWin(TheWinPos);
			break;

		case 'S': SwitchCatFilter(TheFilter.logCat.pos - 1); break;
		case 's': SwitchCatFilter(TheFilter.logCat.pos + 1); break;

		case 'R': SwitchLblFilter(0); SwitchCatFilter(0); DeleteIdleHosts(); // fall through
		case 'r': clearok(curscr, true); SwitchWin(TheWinPos); break;

		case 'q':
		case 'Q': DoShutdown = true; break;

		default: return;
	}

	if (ShowingHelp)
		ShowingHelp = false;

	wrefresh(TheWins[TheWinPos]->handle);
}

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
		if (const Error err = Error::LastExcept(EWOULDBLOCK)) {
			cerr << "failed to read from dispatcher at " << TheDisp << ": " << err << endl;
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

	wrefresh(TheWins[TheWinPos]->handle);
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

	sleepFor(thePeriod);
}


/* Host */

Host::Host(const NetAddr &anAddr): theAddr(anAddr), theLog(2), isSelected(false) {
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


/* WinMatrix */

WinMatrix::WinMatrix(int aMaxY, int aMaxX): theMaxY(aMaxY), theMaxX(aMaxX) {
	theImage.resize((aMaxY+1) * (aMaxX+1));
}

int WinMatrix::safePos(int y, int x) const {
	Assert(y <= theMaxY && x <= theMaxX);
	const int p = y*(theMaxX+1) + x;
	Assert(p < theImage.count());
	return p;
}

/* Window */

Window::Window(const char *aTitle): handle(newwin(0,0,0,0)), title(aTitle) {
	leaveok(handle, true); // do not bother about cursor pos

	int maxx, maxy;
	(void)maxy; // silence g++ "set but not used" warning
	getmaxyx(handle, maxy, maxx);

	werase(handle);

	// print window title
	wattron(handle, A_BOLD);
	wattron(handle, A_UNDERLINE);
	mvwaddstr(handle, 0, (maxx - strlen(title))/2, (char*)title);
	wattroff(handle, A_UNDERLINE);
	wattroff(handle, A_BOLD);
}

Window::~Window() {
	delwin(handle);
}


/* InfoWindow */

InfoWindow::InfoWindow(char const *aTitle): Window(aTitle),
	theMsgCnt(0), theListnCnt(0) {
	update();
}

void InfoWindow::update() {
	const int x = 2;
	int y = 3;
	mvwprintw(handle, y++, x, (char*)"%20s %s:%d", "dispatcher:", TheDisp.addrA().cstr(), TheDisp.port());
	mvwprintw(handle, y++, x, (char*)"%20s %7d", "listeners:", theListnCnt);
	y++;
	mvwprintw(handle, y++, x, (char*)"%20s %7d", "messages received:", theMsgCnt);
	mvwprintw(handle, y++, x, (char*)"%20s %7d", "mean message size:", sizeof(StatusNotifMsg));
	mvwprintw(handle, y++, x, (char*)"%20s %7d", "selected hosts:", TheSelHostCnt);
	mvwprintw(handle, y++, x, (char*)"%20s %7d", "busy hosts:", TheBusyHostCnt);
	mvwprintw(handle, y++, x, (char*)"%20s %7d", "experiments:", TheUniqLblCnt);
}

void InfoWindow::noteAdd(const Host &host) {
	Window::noteAdd(host);
	update();
}

void InfoWindow::noteDel(const Host &host) {
	Window::noteDel(host);
	update();
}

void InfoWindow::noteUpd(const Host &host) {
	Window::noteUpd(host);
	if (host.log().depth()) {
		theMsgCnt++;
		theListnCnt = host.log()[0].theCopyCnt;
	}
	update();
}


/* HelpWindow */

HelpWindow::HelpWindow(): Window("Help") {
	update();
}

void HelpWindow::update() {
	const int x = 2;
	int y = 2;
	mvwprintw(handle, y++, x, "0:\t\tMonitor Info.");
	mvwprintw(handle, y++, x, "LEFT/RIGHT:\tNext/previous tab.");
	mvwprintw(handle, y++, x, "UP/DOWN:\tChange label filters.");
	mvwprintw(handle, y++, x, "h:\t\tShow/close this help tab.");
	mvwprintw(handle, y++, x, "S/s:\t\tChange client/server-side filters.");
	mvwprintw(handle, y++, x, "R:\t\tReset filters, remove idle hosts, and refresh screen.");
	mvwprintw(handle, y++, x, "r:\t\tRefresh screen.");
	mvwprintw(handle, y++, x, "Q/q:\t\tQuit.");
}


/* SumWindow */

SumWindow::SumWindow(char const *aTitle): Window(aTitle) {
	update();
}

void SumWindow::update() {

	// collect summary info from hosts
	MsgSum cltSum, srvSum;
	for (int i = 0; i < TheHosts.count(); ++i) {
		if (const Host *host = TheHosts[i]) {
			if (host->log().depth()) {
				const StatusFwdMsg &msg = host->log()[0];
				if (host->isClient())
					cltSum += msg;
				else
					srvSum += msg;
			}
		}
	}

	const int x = 2;

	int y = 2;
	mvwprintw(handle, y++, x, (char*)"%s (%d hosts)", "Client side:", cltSum.hostCount());
	displaySide(y, x+2, cltSum);

	y += 1;
	mvwprintw(handle, y++, x, (char*)"%s (%d hosts)", "Server side:", srvSum.hostCount());
	displaySide(y, x+2, srvSum);

	y += 1;
	mvwprintw(handle, y++, x+2, (char*)"%20s %7s %8s %9s %9s %s",
		"measurement:", "min", "mean", "max", "sum", "unit");
}

void SumWindow::displaySide(int &y, int x, const MsgSum &sum) {
	//mvwprintw(handle, y++, x, (char*)"%20s %s", "labels:",
	//	(const char*)sum.theLabels);

	displayLine(y++, x, "load:", sum.theReqRate, "req/sec");
	displayLine(y++, x, "throughput:", sum.theRepRate, "rep/sec");
	//displayLine(y++, x, "bandwidth:", sum.theBwidth, "Mbit/sec", 1024.*1024/8);

	displayLine(y++, x, "response time:", sum.theRespTime, "msec");
	displayLine(y++, x, "DHR:", sum.theDHR, "%");
	//displayLine(y++, x, "connection use:", sum.theConnUse, "xact/conn");

	displayLine(y++, x, "errors total:", sum.theTotErrRatio, "%");
	//displayLine(y++, x, "errors now:", sum.theErrRatio, "%");
	displayLine(y++, x, "xactions total:", sum.theXactTotCnt, "x10^6", 1e6);
	displayLine(y++, x, "open sockets now:", sum.theSockInstCnt, "");
}

void SumWindow::displayLine(int y, int x, const char *label, const AggrStat &stats, const char *meas, double scale) {
	mvwprintw(handle, y, x, (char*)"%20s %7d %8d %9d %9d %s",
		label,
		Dbl2Int(stats.min()/scale),
		Dbl2Int(stats.mean()/scale), 
		Dbl2Int(stats.max()/scale),
		Dbl2Int(stats.sum()/scale),
		meas);
}

void SumWindow::noteAdd(const Host &host) {
	Window::noteAdd(host);
	update();
}

void SumWindow::noteDel(const Host &host) {
	Window::noteDel(host);
	update();
}

void SumWindow::noteUpd(const Host &host) {
	Window::noteUpd(host);
	update();
}


/* SmxWindow */

SmxWindow::SmxWindow(char const *aTitle): Window(aTitle) {

	wattron(handle, A_BOLD);

	// horizontal lables
	for (int i = 0; i < 10; ++i)
		mvwprintw(handle, 2, Id2X(i), (char*)"%7d", i+1);

	// vertical labels
	for (int i = 0; i < 100; i += 10)
		mvwprintw(handle, Id2Y(i), 0, (char*)"%2d", i);

	wattroff(handle, A_BOLD);
}

void SmxWindow::noteAdd(const Host &host) {
	Window::noteAdd(host);
	displayMsg(Host2Y(host), Host2X(host), host.log()[0]);
}

void SmxWindow::noteDel(const Host &host) {
	Window::noteDel(host);
	eraseSlot(Host2Y(host), Host2X(host));
}

void SmxWindow::noteUpd(const Host &host) {
	Window::noteUpd(host);
	displayMsg(Host2Y(host), Host2X(host), host.log()[0]);
}

void SmxWindow::eraseSlot(int y, int x) {
	mvwprintw(handle, y, x, (char*)"%7s ", "");
}

int SmxWindow::Host2Y(const Host &host) {
	return Id2Y((host.lna() & 255) - 1);
}

int SmxWindow::Host2X(const Host &host) {
	return Id2X((host.lna() & 255) - 1);
}

int SmxWindow::Id2Y(int host_id) {
	return TheYMargin + ((host_id % 100) / 10) * TheYCellWidth;
}

int SmxWindow::Id2X(int host_id) {
	return TheXMargin + (host_id % 10) * TheXCellWidth;
}


/* NumxWindow */

NumxWindow::NumxWindow(const char *aTitle, const char *aFmt):
	SmxWindow(aTitle), theFmt(aFmt), theMatrix(Id2Y(99),Id2X(99)),
	theNumSum(0), theNumCnt(0) {

	update();
}

void NumxWindow::noteAdd(const Host &host) {
	theNumCnt++;
	SmxWindow::noteAdd(host);
}

void NumxWindow::noteDel(const Host &host) {
	theNumCnt--;
	SmxWindow::noteDel(host);
}

void NumxWindow::noteUpd(const Host &host) {
	SmxWindow::noteUpd(host);
}

void NumxWindow::displayMsg(int y, int x, const StatusFwdMsg &msg) {
	const double n = msg2num(msg);
	theNumSum -= theMatrix(y, x);
	mvwprintw(handle, y, x, (char*)theFmt, n);
	theMatrix(y, x) = n;
	theNumSum += n;
	update();
}

void NumxWindow::eraseSlot(int y, int x) {
	theNumSum -= theMatrix(y, x);
	SmxWindow::eraseSlot(y, x);
	theMatrix(y, x) = 0;
	update();
}

void NumxWindow::update() {
	int maxx, maxy;
	getmaxyx(handle, maxy, maxx);

    int y = maxy-1;

	wattron(handle, A_BOLD);
	mvwaddstr(handle, y, Id2X(2) + TheXCellWidth-4, (char*)"cnt:");
	mvwaddstr(handle, y, Id2X(4) + TheXCellWidth-4, (char*)"avg:");
	mvwaddstr(handle, y, Id2X(6) + TheXCellWidth-4, (char*)"sum:");
	wattroff(handle, A_BOLD);

	mvwprintw(handle, y, Id2X(3), (char*)"%7d", theNumCnt);
	mvwprintw(handle, y, Id2X(5), (char*)theFmt, Ratio(theNumSum,theNumCnt));
	mvwprintw(handle, y, Id2X(7), (char*)theFmt, theNumSum);
}


/* RunTimeSmxWin */

void RunTimeSmxWin::displayMsg(int y, int x, const StatusFwdMsg &msg) {
	const int sec = (msg.theSndTime - msg.theStartTime).sec();
	const int min = (sec/60) % 60;
	const int hour = sec/3600;
	if (hour > 0)
		mvwprintw(handle, y, x, (char*)"%4d:%02d", hour, min);
	else
		mvwprintw(handle, y, x, (char*)"%4d.%02d", min, sec%60);
}


/* MsgGapSmxWin */

void MsgGapSmxWin::noteUpd(const Host &host) {
	if (host.log().depth() > 1)
		theLastMsg = host.log()[1];
	SmxWindow::noteUpd(host);
}

void MsgGapSmxWin::displayMsg(int y, int x, const StatusFwdMsg &msg) {
	if (theLastMsg.theRcvTime >= 0)
		mvwprintw(handle, y, x, (char*)"%7d",
			(msg.theRcvTime - theLastMsg.theRcvTime).sec());
	else
		mvwprintw(handle, y, x, (char*)"%7s", "?");
}

MsgSum::MsgSum(): theLabels("") {
}

MsgSum &MsgSum::operator +=(const StatusFwdMsg &msg) {
	/*if (msg.theLabel && !theLabels.str(msg.theLabel)) { // XXX: not sufficient
		if (theLabels)
			theLabels += ",";
		theLabels += msg.theLabel;
	}*/

	theReqRate.record(Dbl2Int(msg.theReqRate));
	theRepRate.record(Dbl2Int(msg.theRepRate));
	theBwidth.record(Dbl2Int(msg.theBwidth));
	theRespTime.record(msg.theRespTime.msec());
	theDHR.record(Dbl2Int(msg.theDHR*100));
	theConnUse.record(Dbl2Int(msg.theConnUse));
	theErrRatio.record(Dbl2Int(msg.theErrRatio*100));
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

	SelectHost(host); // check select status

	Broadcast(&Window::noteUpd, *host);
}

static
void Broadcast(Window::EventHandler weh, const Host &host) {
	if (!host.selected())
		(TheInfoWin->*weh)(host); // info window gets all events
	else
		for (int w = 0; w < TheWins.count(); ++w)
			(TheWins[w]->*weh)(host);
}

#if 0
static
void Broadcast(Window::EventHandler weh) {
	for (int h = 0; h < TheHosts.count(); ++h) {
		if (TheHosts[h])
			Broadcast(weh, *TheHosts[h]);
	}
}
#endif


static
void DeleteIdleHosts() {
	// we will these from scratch:
	TheHostIdx.clear();
	TheSelHostCnt = 0;

	for (int h = 0; h < TheHosts.count(); ++h) {
		if (Host *host = TheHosts[h]) {
			if (host->busy()) {
				// XXX pre-IPv6 code used 's_addr' as an int
				const int idx = host->addr().addrN().octet(0) % TheHostIdx.count();
				TheHostIdx[idx] = host;
				if (host->selected())
					TheSelHostCnt++;
			} else {
				TheBusyHostCnt--;
				Broadcast(&Window::noteDel, *host);
				DelLastLabel(host, host->runLabel());
				delete host;
				TheHosts[h] = 0;
			}
		}
	}
}

static
bool AddFirstLabel(const Host *skip, const String &l) {
	if (!l)
		return false; // empty label

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

static
int SwitchWin(int idx) {
	TheWinPos = (idx + TheWins.count()) % TheWins.count();
	touchwin(TheWins[TheWinPos]->handle);
	return TheWinPos;
}

static
void SelectHost(Host *host) {
	const bool wasSel = host->selected();
	const bool isSel = host->matches(TheFilter);
	if (!wasSel && isSel) {
		host->selected(isSel);
		TheSelHostCnt++;
		Broadcast(&Window::noteAdd, *host);
	} else
	if (wasSel && !isSel) {
		TheSelHostCnt--;
		Broadcast(&Window::noteDel, *host);
		host->selected(isSel);
	}
}

static
void SelectHosts() {
	for (int h = 0; h < TheHosts.count(); ++h) {
		if (Host *host = TheHosts[h])
			SelectHost(host);
	}

	// update selection warnings
	ofixedstream s(TheFiltWarn, sizeof(TheFiltWarn));
	BuildFiltWarn(s);
	for (int w = 0; w < TheWins.count(); ++w) {
		if (strlen(TheFiltWarn)) {
			wattron(TheWins[w]->handle, A_BOLD);
			mvwaddstr(TheWins[w]->handle, 1, 0, (char*)"Filters:");
			wattroff(TheWins[w]->handle, A_BOLD);
			mvwprintw(TheWins[w]->handle, 1, 10, (char*)"%-20s", TheFiltWarn);
		} else {
			wattroff(TheWins[w]->handle, A_BOLD);
			mvwprintw(TheWins[w]->handle, 1, 0, (char*)"%-30s", "");
		}
	}

	touchwin(TheWins[TheWinPos]->handle);
}

static
int SwitchLblFilter(int idx) {
	TheFilter.lbl.pos = (idx + TheLabels.count()) % TheLabels.count();

	if (TheFilter.lbl.pos)
		for (int i = 0; !*TheLabels[TheFilter.lbl.pos] && i < TheLabels.count(); ++i) {
			TheFilter.lbl.pos++;
			TheFilter.lbl.pos %= TheLabels.count();
		}

	SelectHosts();
	return TheFilter.lbl.pos;
}

static
int SwitchCatFilter(int idx) {
	TheFilter.logCat.pos = (idx + lgcEnd) % lgcEnd;
	SelectHosts();
	return TheFilter.logCat.pos;
}

static
void BuildFiltWarn(ostream &os) {
	if (TheFilter.lbl.pos) {
		Assert(*TheLabels[TheFilter.lbl.pos]);
		os << *TheLabels[TheFilter.lbl.pos];
	}

	if (TheFilter.logCat.pos) {
		if (TheFilter.lbl.pos)
			os << ',';
		os << (TheFilter.logCat.pos == lgcCltSide ?
			"clt" : "srv");
	}
	os << ends;
}

int main(int argc, char *argv[]) {
	(void)PolyVersion();

	if (argc == 2 && String("--help") == argv[1]) {
		cout << "Usage: " << argv[0]
			<< " [--help] [udp2tcpd_ip [udp2tcpd_port]]"
			<< endl;
		return 0;
	}

	const char *disph = argc >= 2 ? argv[1] : "127.0.0.1";
	const int dispp = argc >= 3 ? atoi(argv[2]) : 18256;
	TheDisp = NetAddr(disph, dispp);

	Socket sock;
	Must(sock.create(TheDisp.addrN().family()));
	if (!Should(sock.connect(TheDisp))) {
		cerr << "failed to connect to udp2tcp dispatcher at " << TheDisp << endl;
		return -2;
	}
	Must(sock.blocking(false));

	Must(initscr());
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr,false);
	keypad(stdscr,true);
	curs_set(0);
	wtimeout(stdscr, 0); // delay for cusrses input, msec; zero == nonblocking

	TheHostIdx.resize(256);
	TheLabels.append(new String);

	TheWins.append(TheInfoWin = new InfoWindow("Monitor Info"));
	TheWins.append(new SumWindow("Summary Information"));
	TheWins.append(new RunLabelSmxWin("Experiment Label"));
	TheWins.append(new RunTimeSmxWin("Run time [h:m.s]"));
	TheWins.append(new ReqRateSmxWin("Load [requests/sec]"));
	TheWins.append(new RepRateSmxWin("Throughput [replies/sec]"));
	TheWins.append(new BwidthSmxWin("Network Bandwidth [Mbps,replies]"));
	TheWins.append(new RespTimeSmxWin("Response Time [msec]"));
	TheWins.append(new DHRSmxWin("DHR [%]"));
	TheWins.append(new ConnUseSmxWin("Connection Use [xact/conn]"));
	TheWins.append(new ErrTotRatioSmxWin("Errors Total [%]"));
	TheWins.append(new ErrTotCntSmxWin("Errors Total Count x 1000"));
	TheWins.append(new XactTotCntSmxWin("Total Xaction Count x 1000"));
	TheWins.append(new ErrRatioSmxWin("Errors Now [%]"));
	TheWins.append(new SockInstCntSmxWin("Open Sockets Now"));
	TheWins.append(new MsgGapSmxWin("Message Gap [sec]"));

	TheHelpWin = new HelpWindow;

	signal(SIGPIPE, SIG_IGN);
	Clock::Update();

	TheScanner = new PG_PREFFERED_FILE_SCANNER;
	TheScanner->configure();

	MsgMonitor *msgSrv = new MsgMonitor(sock);
	KbdMonitor *kbdSrv = new KbdMonitor(0); // XXX: how to get stdin fd?
	Ticker *ticker = new Ticker(Time::Sec(60));

	SwitchWin(0);
	wrefresh(TheWins[TheWinPos]->handle);

	while (!DoShutdown) {
		Clock::Update();
		Time tout = TheAlarmClock.on() ? 
			TheAlarmClock.timeLeft() : Time();
		TheScanner->scan(TheAlarmClock.on() ? 0 : &tout);
	}

	delete ticker;
	delete msgSrv;
	delete kbdSrv;
	delete TheHelpWin;
	delete TheScanner;

	endwin();

	return 0;
}


#endif /* HAVE_NCURSES_H */
