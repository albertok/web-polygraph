
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/os_std.h"
#include "xstd/h/string.h"
#include "xstd/h/signal.h"
#include "xstd/h/netinet.h"
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Assert.h"
#include "xstd/Socket.h"
#include "xstd/Clock.h"
#include "xstd/Select.h"
#include "xstd/Poll.h"
#include "xstd/Epoll.h"
#include "xstd/gadgets.h"
#include "base/polyVersion.h"
#include "runtime/NotifMsg.h"


static String ThePrgName;

// maintains info about a host
class Client: public FileScanUser {
	public:
		Client(Socket aFD, const NetAddr &anAddr);
		virtual ~Client();

		void append(const void *buf, int size);

		virtual void noteWriteReady(int fd);
		
	protected:
		void selfDistruct();

	protected:
		Socket theSock;
		NetAddr theAddr; // unique host address
		FileScanReserv theReserv;
		char theBuf[1024];
		int theSize;
};

class TcpServer: public FileScanUser {
	public:
		TcpServer(int port);
		virtual ~TcpServer();

		virtual void noteReadReady(int fd);

	protected:
		Socket theSock;
		FileScanReserv theReserv;
};

class UdpServer: public FileScanUser {
	public:
		UdpServer(int port);
		virtual ~UdpServer();

		virtual void noteReadReady(int fd);

	protected:
		Socket theSock;
		FileScanReserv theReserv;
};


/* globals */

static Array<Client*> TheClients;
static FileScanner *TheScanner = 0;

static int TheLiveCltCnt = 0;

/* implementation */


/* Client */

Client::Client(Socket aFD, const NetAddr &anAddr): 
	theSock(aFD), theAddr(anAddr), theSize(0) {
	theSock.blocking(false);
	TheLiveCltCnt++;
}

Client::~Client() {
	if (theReserv)
		TheScanner->clearRes(theReserv);
	TheLiveCltCnt--;
}

void Client::append(const void *buf, int size) {
	if (size + theSize > (int)sizeof(theBuf)) {
		selfDistruct();
		return;
	}
	memcpy(theBuf + theSize, buf, size);
	theSize += size;
	if (!theReserv)
		theReserv = TheScanner->setFD(theSock.fd(), dirWrite, this);
}

void Client::noteWriteReady(int) {
	if (theSize) {
		const int sz = theSock.write(theBuf, theSize);
		if (sz < 0) {
			if (Error::LastExcept(EWOULDBLOCK))
				selfDistruct();
			return;
		}
		theSize -= sz;
		memmove(theBuf, theBuf+sz, theSize);
	}

	if (!theSize)
		TheScanner->clearRes(theReserv);
}

void Client::selfDistruct() {
	for (int i = 0; i < TheClients.count(); ++i)
		if (TheClients[i] == this) {
			TheClients[i] = 0;
			delete this;
			return;
		}
	Assert(0);
}


/* TcpServer */

TcpServer::TcpServer(int port) {
	Must(theSock.create(InAddress::IPvFour().family()));
	Must(theSock.blocking(false));
	theSock.reuseAddr(true);
	// XXX relies on InAddress class to zero address value, making it
	// equivalent to INADDR_ANY
	if (!theSock.bind(NetAddr(InAddress::IPvFour(), port)) || !theSock.listen()) {
		cerr << "cannot listen on TCP port " << port << ": " << Error::Last() << endl;
		exit(-2);
	}

	theReserv = TheScanner->setFD(theSock.fd(), dirRead, this);
}

TcpServer::~TcpServer() {
	if (theReserv)
		TheScanner->clearRes(theReserv);
}

void TcpServer::noteReadReady(int) {
	sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	if (Socket s = theSock.accept((struct sockaddr*)&addr, &addr_len)) {
		Client *c = new Client(s, NetAddr(
			((sockaddr_in &)addr).sin_addr,
			((sockaddr_in &)addr).sin_port));

		for (int i = 0; i < TheClients.count(); ++i)
			if (!TheClients[i]) {
				TheClients[i] = c;
				return;
			}
		TheClients.append(c);
	}
}


/* UdpServer */

UdpServer::UdpServer(int port) {
	// XXX hard-code IPv4
	Must(theSock.create(PF_INET, SOCK_DGRAM, 0));
	theSock.reuseAddr(true);
	// XXX relies on InAddress class to zero address value, making it
	// equivalent to INADDR_ANY
	if (!theSock.bind(NetAddr(InAddress::IPvFour(), port))) {
		cerr << "cannot listen on UDP port " << port << ": " << Error::Last() << endl;
		exit(-2);
	}
	Must(theSock.blocking(false));

	theReserv = TheScanner->setFD(theSock.fd(), dirRead, this);
}

UdpServer::~UdpServer() {
	if (theReserv)
		TheScanner->clearRes(theReserv);
}

void UdpServer::noteReadReady(int) {
	StatusNotifMsg msg;
	struct sockaddr_storage from;
	socklen_t fromlen = sizeof(from);
	if (recvfrom(theSock.fd(), (char*)&msg, sizeof(msg), 0, (sockaddr *) &from, &fromlen) == sizeof(msg)) {
		msg.ntoh();
		StatusFwdMsg fwd(msg, TheClock, NetAddr(from, -1));
		fwd.theCopyCnt = TheLiveCltCnt;
		fwd.hton();
		for (int i = 0; i < TheClients.count(); ++i)
			if (TheClients[i])
				TheClients[i]->append(&fwd, sizeof(fwd));
	}
}

static
int usage(std::ostream &os) {
	(void)PolyVersion();
	os << "Usage: " << ThePrgName << " [udp_and_tcp_port]" << endl;
	return 0;
}

int main(int argc, char *argv[]) {
	ThePrgName = argv[0];

	if (argc == 2 && String("--help") == argv[1])
		return usage(cout);

	// note: we assume that TCP and UDP port namespaces are separate
	if (argc > 2 || (argc == 2 && !xatoi(argv[1]))) {
		usage(cerr);
		return -1;
	}

	const int port = xatoi(argv[1], 18256);

	cout << "Starting UDP to TCP dispatcher on port " << port << endl;

	// detach ourselves if possible
#if HAVE_FORK
	const pid_t pid = fork();

	if (pid > 0) // parent
		return 0;

    if (pid < 0 || setsid() < 0) { // error
        cerr << "problems detaching the server process: " << Error::Last() << endl;
		return -1;
	}
#endif

	TheScanner = new PG_PREFFERED_FILE_SCANNER;
	TheScanner->configure();

	TcpServer *tcpSrv = new TcpServer(port);
	UdpServer *udpSrv = new UdpServer(port);

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	while (!TheScanner->idle()) {
		Clock::Update();
		TheScanner->scan(0);
	}

	delete udpSrv;
	delete tcpSrv;
	delete TheScanner;

	return 0;
}
