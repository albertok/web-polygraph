
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#define INET6 1

#include "xstd/xstd.h"

#include <ctype.h>
#include "xstd/Assert.h"
#include "xstd/Socket.h"

#include "xstd/h/os_std.h"
#include "xstd/h/fcntl.h"
#include "xstd/h/net/if.h"
#include "xstd/h/netinet.h"
#include "xstd/h/sys/sockio.h"  /* for SIOCGIFCONF on Solaris */
#include "xstd/h/sys/ioctl.h"
#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"

#include "xstd/InetIfReq.h"

#if HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

bool Socket::IsEnabled = true;
int  Socket::TheMaxLevel = -1;
int  Socket::TheLevel = 0;

#if HAVE_SA_LEN
#define SOCKADDRLEN_SS(A) A.ss_len
#define SOCKADDRLEN_SIN(A) A.sin_len
#else
#define SOCKADDRLEN_SS(A) ((A).ss_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6))
#define SOCKADDRLEN_SIN(A) sizeof(A)
#endif


void Socket::Configure() {
#ifdef HAVE_WSASTARTUP
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	Must(WSAStartup(wVersionRequested, &wsaData) == 0);
	/* Confirm that the WinSock DLL supports 2.2.*/
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup( );
		Must(false);
	}
#endif
}

void Socket::Clean() {
#ifdef HAVE_WSACLEANUP
	WSACleanup();
#endif
}

void Socket::TooManyFiles() {
#ifdef WSAEMFILE
	Error::Last(WSAEMFILE);
#else
	Error::Last(EMFILE);
#endif
}

// called when theFd is set to a new value
bool Socket::noteNewFD() {
	if (theFD < 0)
		return false;

	TheLevel++;

	// most MaxLevel users assume that MaxLevel means max FD!
	if (TheMaxLevel >= 0 && theFD >= TheMaxLevel) {
		close();
		TooManyFiles();
		return false;
	}

	return true;
}

bool Socket::create(int family) {
	return create(family, SOCK_STREAM, 0);
}

bool Socket::create(int domain, int type, int protocol) {

	if (TheMaxLevel >= 0 && TheLevel >= TheMaxLevel) {
		TooManyFiles();
		return false;
	}
	
	theFD = ::socket(domain, type, protocol);
	return noteNewFD();
}

bool Socket::bind(const NetAddr &addr) {
	const sockaddr_storage s = addr.sockAddr();
	//cerr << "binding FD " << theFD << " to " << addr << endl;
	return !IsEnabled ||
		::bind(theFD, (sockaddr *) &s, SOCKADDRLEN_SS(s)) == 0;
}

bool Socket::connect(const NetAddr &addr) {
	const sockaddr_storage s = addr.sockAddr();
	if (IsEnabled && ::connect(theFD, (sockaddr *) &s, SOCKADDRLEN_SS(s)) < 0) {
		const Error err = Error::Last();
		return err == EINPROGRESS || err == EWOULDBLOCK;
	}
	return true;
}

bool Socket::listen() {
	return !IsEnabled ||
		::listen(theFD, FD_SETSIZE) == 0;
}

Socket Socket::accept(sockaddr *addr_p, socklen_t *addr_len_p) {

	if (TheMaxLevel >= 0 && TheLevel >= TheMaxLevel) {
		TooManyFiles();
		return -1;
	}

	sockaddr addr;
	socklen_t addr_len = sizeof(addr);
	if (!addr_p) {
		addr_p = &addr;
		addr_len_p = &addr_len;
	} else
		Assert(addr_len_p);

	const int fd = IsEnabled ?
		::accept(theFD, addr_p, addr_len_p) :
		(Error::Last(EWOULDBLOCK), -1);

	Socket s(fd);
	s.noteNewFD();
	return s;
}

Socket Socket::accept(NetAddr &addr) {
	struct sockaddr_storage addr_in;
	memset(&addr_in, 0, sizeof(addr_in));
	socklen_t addr_len = sizeof(addr_in);
	const Socket s = accept((sockaddr*)&addr_in, &addr_len);
	if (s) {
		addr.addr(addr_in);
		addr.port(InAddress::GetPort(addr_in));
	}
	return s;
}

Size Socket::read(void *buf, Size sz) {
	return IsEnabled && sz > 0 ? 
		sysRead(buf, sz) : (Size)0;
}

Size Socket::write(const void *buf, Size sz) {
	return IsEnabled && sz > 0 ? 
		sysWrite(buf, sz) : sz;
}

Size Socket::recvFrom(void *buf, Size sz, NetAddr &addr, int flags) {
	if (!IsEnabled || sz <= 0)
		return 0;

	sockaddr_storage s = addr.sockAddr();
	socklen_t addr_len = SOCKADDRLEN_SS(s);

	// must cast to char* on Solaris
	const Size readSz =
		(Size)::recvfrom(theFD, (char*)buf, sz, flags, (sockaddr *) &s, &addr_len);

	if (readSz >= 0) {
		addr.addr(s);
		addr.port(InAddress::GetPort(s));
	}
	return readSz;
}

Size Socket::sendTo(const void *buf, Size sz, const NetAddr &addr, int flags) {
	sockaddr_storage s = addr.sockAddr();

	// must cast to char* on Solaris
	return IsEnabled && sz > 0 ?
		(Size)::sendto(theFD, (const char *)buf, sz, flags, (sockaddr *) &s, SOCKADDRLEN_SS(s)) :
		(Size)0;
}

bool Socket::close() {
	if (theFD >= 0) {
		const bool res = sysClose();
		theFD = -1;
		TheLevel--;
		return res;
	}
	Error::Last(EBADF);
	return false;
}

bool Socket::sendBuf(Size sz) {
#ifdef SO_SNDBUF
	return setOpt(SO_SNDBUF, (const void*)&sz, sizeof(sz));
#else
	return unsupOpt(&sz);
#endif
}

bool Socket::recvBuf(Size sz) {
#ifdef SO_RCVBUF
	return setOpt(SO_RCVBUF, (const void*)&sz, sizeof(sz));
#else
	return unsupOpt(&sz);
#endif
}

bool Socket::getIfConf(struct ifconf &cfg) {
#ifdef SIOCGIFCONF
	return ioctl(theFD, SIOCGIFCONF, &cfg) >= 0;
#elif defined(SIO_GET_INTERFACE_LIST) && 0 
	// XXX: fix me, see getIfAddrs.cc
	const int iiCount = cfg.ifc_len / sizeof(ifreq);
	INTERFACE_INFO iis[iiCount];
	unsigned long iiLen = 0;
	// XXX: treat WSAEFAULT as required by WSAIoctl docs!
	if (WSAIoctl(theFD, SIO_GET_INTERFACE_LIST, 0, 0, &iis,
		sizeof(iis), &iiLen, 0, 0) == -1)
		return false;

	const int iCount = iiLen / sizeof(INTERFACE_INFO);
	Assert(iCount * sizeof(ifreq) <= cfg.ifc_len);
	cfg.ifc_len = iCount * sizeof(ifreq);
	for (int n = 0; n < iCount; ++n) {
		INTERFACE_INFO *i = iis + n;
		sockaddr_in* addr; = (sockaddr_in*)&(i->iiAddress);

		// set ifreq fields
		ifreq *req = (ifreq*)(cfg.ifc_buf + n*sizeof(ifreq));
		strncpy(req->ifr_name, i->name, sizeof(req->ifr_name));
		req->ifr_addr = addr;
	}
	return true;
#else
	Error::Last(EINVAL);
	return false;
#endif
}

bool Socket::getV4IfAddr(const String &ifname, InAddress &addr) {
#ifdef SIOCGIFADDR
	struct ifreq r;
	strncpy(r.ifr_name, ifname.cstr(), sizeof(r.ifr_name));
	if (0 == ioctl(theFD, SIOCGIFADDR, &r)) {
		addr = InAddress(r.ifr_addr);
		return true;
	}
	return false;
#endif
	Error::Last(EINVAL);
	return false;
}

// Find the "primary" IPv6 address of an interface.
// We currently define "primary" as the address added to the interface first.
bool Socket::getV6IfAddr(const String &ifname, InAddress &addr) {
#if HAVE_GETIFADDRS
	struct ifaddrs *ifp = NULL;
	if (getifaddrs(&ifp) < 0)
		return false;
	bool found = false;
	for (struct ifaddrs *p = ifp; p; p = p->ifa_next) {
		if (!p->ifa_addr ||
			p->ifa_addr->sa_family != AF_INET6)
			continue;
		if (ifname != p->ifa_name)
			continue;
		addr = InAddress(*p->ifa_addr);
		found = true;
		// Linux uses LIFO: The address added first is the last in the list.
		// FreeBSD (and others?) use FIFO; we break on first hit for them.
#ifndef __linux__
		break;
#endif
	}
	freeifaddrs(ifp);
	if (!found)
		Error::Last(ENOENT);
	return found;
#else
	Error::Last(EINVAL);
	return false;
#endif
}

bool Socket::getIfBcastAddr(const String &ifname, struct sockaddr &addr) {
#ifdef SIOCGIFBRDADDR
	struct ifreq r;
	strncpy(r.ifr_name, ifname.cstr(), sizeof(r.ifr_name));
	if (ioctl(theFD, SIOCGIFBRDADDR, &r) < 0)
		return false;
	addr = r.ifr_addr;
	return true;
#else
// XXX need IPv6 counterpart?
	Error::Last(EINVAL);
	return false;
#endif
}

bool Socket::addV4IfAddr(const InetIfAliasReq &req, int idx) {
#if defined(SIOCAIFADDR)
	Assert(sizeof(idx));
	return ioctl(theFD, SIOCAIFADDR, &req) >= 0;
#elif defined(SIOCSIFADDR)
	// some OSes use "ifname:alias_idx" format for aliases
	// we assume that same OSes cannot pass bcast and mask
	// inside one ioctl request
	InetIfAliasReq r(req);

	if (numberedIface(r, idx)) {
		r.addr(req.addr());
		if (ioctl(theFD, SIOCSIFADDR, &r) < 0)
			return false;

		r.addr(req.broad());
		if (ioctl(theFD, SIOCSIFBRDADDR, &r) < 0)
			return false;

		r.addr(req.mask());
		if (ioctl(theFD, SIOCSIFNETMASK, &r) < 0)
			return false;

		return true;
	}
	Error::Last(EINVAL);
	return false;
#else
	Assert(sizeof(idx) && sizeof(&req));
	Error::Last(EINVAL);
	return false;
#endif
}

bool Socket::addV6IfAddr(const Inet6IfAliasReq &req, int idx) {
	Assert(sizeof(idx));
#if defined(SIOCAIFADDR_IN6)
	return ioctl(theFD, SIOCAIFADDR_IN6, &req) >= 0;
#elif defined(SIOCSIFADDR)
	return ioctl(theFD, SIOCSIFADDR, &req) >= 0;
#else
	Assert(sizeof(&req));
	Error::Last(EINVAL);
	return false;
#endif
}

bool Socket::delV4IfAddr(const InetIfAliasReq &req, int idx) {
#if !defined(SIOCAIFADDR) && defined(SIOCGIFFLAGS) && defined(IFF_UP)
	// some OSes use "ifname:alias_idx" format for aliases
	// linux (and others?) wants us to put the iface down instead
	// of calling SIOCDIFADDR
	InetIfAliasReq r(req);
	if (numberedIface(r, idx)) {
		if (ioctl(theFD, SIOCGIFFLAGS, &r) < 0)
			return false;
		((ifreq&)r).ifr_flags &= ~IFF_UP;
		return ioctl(theFD, SIOCSIFFLAGS, &r) >= 0;
	}
	Error::Last(EINVAL);
	return false;
#elif defined(SIOCDIFADDR)
	Assert(sizeof(idx));
	return ioctl(theFD, SIOCDIFADDR, &req) >= 0;
#else
	Assert(sizeof(idx) && sizeof(&req));
	Error::Last(EINVAL);
	return false;
#endif
}

bool Socket::delV6IfAddr(const Inet6IfAliasReq &req) {
#if defined(SIOCDIFADDR_IN6)
	return ioctl(theFD, SIOCDIFADDR_IN6, &req) >= 0;
#elif defined(SIOCDIFADDR)
	return ioctl(theFD, SIOCDIFADDR, &req) >= 0;
#else
	Assert(sizeof(&req));
	Error::Last(EINVAL);
	return false;
#endif
}

bool Socket::configure(const SockOpt &opt) {
	if (opt.nagle && !tcpDelay(opt.nagle > 0))
		return false;

	if (opt.linger >= 0 && !linger(opt.linger))
		return false;

	return true;
}

int Socket::flags() const {
#if defined(HAVE_FCTL) && defined(F_GETFL)
	return fcntl(theFD, F_GETFL);
#else
	Error::Last(EINVAL);
	return 0;
#endif
}

/* local port */
int Socket::lport() const {
	sockaddr_in s;
	socklen_t len = SOCKADDRLEN_SIN(s);

	if (getsockname(theFD, (sockaddr*)&s, &len) == 0)
		return ntohs(s.sin_port);
	else
		return -1;
}

NetAddr Socket::localAddr() const {
	sockaddr_in S;
	socklen_t len = sizeof(S);
	if (getsockname(theFD, (sockaddr*)&S, &len) == 0)
		return NetAddr(S.sin_addr, ntohs(S.sin_port));
	else
		return NetAddr();
}

/* remote (peer) port */
int Socket::rport() const {
	sockaddr_in s;
	socklen_t len = SOCKADDRLEN_SIN(s);

	if (getpeername(theFD, (sockaddr*)&s, &len) == 0)
		return ntohs(s.sin_port);
	else
		return -1;
}

Time Socket::rcvTimeout() const {
#ifdef SO_RCVTIMEO
	Time t;
	socklen_t len = sizeof(t);
	if (getOpt(SO_RCVTIMEO, (void*)&t, &len))
		return t;
#endif
	return Time();
}

Size Socket::sendBuf() const {
#ifdef SO_SNDBUF
	Size sz;
	socklen_t len = sizeof(sz);
	if (getOpt(SO_SNDBUF, (void*)&sz, &len))
		return sz;
#endif
	return Size();
}

Size Socket::recvBuf() const {
#ifdef SO_RCVBUF
	Size sz;
	socklen_t len = sizeof(sz);
	if (getOpt(SO_RCVBUF, (void*)&sz, &len))
		return sz;
#endif
	return Size();
}

/*
 * Note: Solaris 2.4's socket emulation doesn't allow you
 * to determine the error from a failed non-blocking
 * connect and just returns EPIPE. -- fenner@parc.xerox.com
 */
Error Socket::error() const {
	int en;
	socklen_t len = sizeof(en);

	if (getOpt(SO_ERROR, (void*)&en, &len))
		return Error::Last(en);
	else
		return Error::Last();
}

bool Socket::peerAddr(sockaddr *S, socklen_t &len) const {
	return getpeername(theFD, S, &len) == 0;
}

NetAddr Socket::peerAddr() const {
	sockaddr_in s;
	socklen_t len = sizeof(s);
	if (peerAddr((sockaddr *) &s, len))
		return NetAddr(s.sin_addr, ntohs(s.sin_port));
	else
		return NetAddr();
}

bool Socket::reuseAddr(bool set) {
#ifdef SO_REUSEADDR
	return setOpt(SO_REUSEADDR, set);
#else
	return unsupOpt(&set);
#endif
}

bool Socket::linger(Time delay) {
#ifdef SO_LINGER
	static struct linger l = // are casts OS-specific?
		{ (bool)(delay.sec() > 0), (unsigned short) delay.sec() };
	return setOpt(SO_LINGER, &l, sizeof(l));
#else
	return unsupOpt(&delay);
#endif
}

bool Socket::blocking(bool block) {
#ifdef FIONBIO
	unsigned long param = !block;
	return ioctl(theFD, FIONBIO, &param) == 0;
#else
	const int oldFlags = flags();
	int newFlags = oldFlags;

	if (block)
		newFlags &= ~O_NONBLOCK;
	else
		newFlags |=  O_NONBLOCK;

	if (newFlags == oldFlags)
		return true;

	return fcntl(theFD, F_SETFL, newFlags) != -1;
#endif
}

bool Socket::reusePort(bool set) {
#ifdef SO_REUSEPORT
	return setOpt(SO_REUSEPORT, set);
#else
	return unsupOpt(&set);
#endif
}

bool Socket::tcpDelay(bool set) {
#ifdef TCP_NODELAY
	int optVal = (int)!set;
	// must cast to char* on Solaris
	return ::setsockopt(theFD, IPPROTO_TCP, TCP_NODELAY, (const char*)&optVal, sizeof(optVal)) == 0;
#else
	return unsupOpt(&set);
#endif
}

bool Socket::isBlocking() const {
#ifdef FIONBIO
	int nb = 0;
	socklen_t len = sizeof(nb);
	return getOpt(FIONBIO, &nb, &len) ? !nb : true;
#else
	return flags() & O_NONBLOCK == 0;
#endif
}


bool Socket::setOpt(int optName, bool set) {
	int optVal = (int)set;
	return setOpt(optName, &optVal, sizeof(optVal));
}

bool Socket::setOpt(int optName, const void *optVal, socklen_t optLen) {
	return setOpt(SOL_SOCKET, optName, (const char*)optVal, optLen);
}

bool Socket::getOpt(int optName, void *optVal, socklen_t *optLen) const {
	return getOpt(SOL_SOCKET, optName, (char*)optVal, optLen);
}

bool Socket::setOpt(int level, int optName, const void *optVal, socklen_t optLen) {
	// must cast to char* on Solaris
	return ::setsockopt(theFD, level, optName, (const char*)optVal, optLen) == 0;
}

bool Socket::getOpt(int level, int optName, void *optVal, socklen_t *optLen) const {
	// must cast to char* on Solaris
	return ::getsockopt(theFD, level, optName, (char*)optVal, optLen) == 0;
}

// W2K do not have read/write/close for sockets, only files
Size Socket::sysRead(void *buf, Size sz) {
#	ifdef HAVE_CLOSESOCKET
		return (Size)::recv(theFD, (char*)buf, sz, 0);
#	else
		return (Size)::read(theFD, buf, sz);
#	endif
}

Size Socket::sysWrite(const void *buf, Size sz) {
#	ifdef HAVE_CLOSESOCKET
		return (Size)::send(theFD, (const char*)buf, sz, 0);
#	else
		return (Size)::write(theFD, buf, sz);
#	endif
}

bool Socket::sysClose() {
#	ifdef HAVE_CLOSESOCKET
		return !::closesocket(theFD);
#	else
		return !::close(theFD);
#	endif
}

bool Socket::numberedIface(InetIfAliasReq &r, int idx) const {
	if (strchr(r.name(), ':')) {
		return true;
	}
	
	if (idx >= 0) {
		const int len = strlen(r.name()); // assume zero-termination
		ofixedstream os(r.name() + len, sizeof(r.name()) - len);
		os << ':' << idx << ends;
		return (bool)os; // no overflows
	}

	return false;
}

// the argument is used by the caller to prevent "arg not used" warnings
bool Socket::unsupOpt(void *) const {
	Error::Last(ENOPROTOOPT);
	return false;
}
