
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_SOCKET_H
#define POLYGRAPH__XSTD_SOCKET_H

#include "xstd/h/sys/socket.h"

#include "xstd/Error.h"
#include "xstd/Time.h"
#include "xstd/Size.h"
#include "xstd/NetAddr.h"
#include "xstd/Array.h"

class InetIfAliasReq;
class Inet6IfAliasReq;
struct ifconf;

// socket option holder
class SockOpt {
	public:
		SockOpt(): nagle(0) {}

	public:
		Time linger;  // linger delay
		int nagle;    // 3-way bool
		/* more to come */
};

// socket wrapper
class Socket {
	public:
		static void Configure();
		static void Clean();
		static int Level() { return TheLevel; }
		static void TooManyFiles();
		static bool IsEnabled;  // bind,connect,listen,read,write calls are enabled
		static int TheMaxLevel; // will not allow Level to go higher

	public:
		Socket(): theFD(-1) {}
		Socket(int aFD): theFD(aFD) {}
		~Socket() {}

		bool create(int family);
		bool create(int domain, int type, int protocol);
		bool connect(const NetAddr &addr);
		bool bind(const NetAddr &addr);
		bool listen();
		Socket accept(struct sockaddr *addr, socklen_t *addr_len);
		Socket accept(NetAddr &addr);
		Size read(void *buf, Size sz);
		Size write(const void *buf, Size sz);
		Size recvFrom(void *buf, Size sz, NetAddr &addr, int flags = 0);
		Size sendTo(const void *buf, Size sz, const NetAddr &addr, int flags = 0);
		bool close();

		bool reuseAddr(bool set);
		bool reusePort(bool set);
		bool linger(Time delay);
		bool blocking(bool set);
		bool tcpDelay(bool set);
		bool sendBuf(Size size);
		bool recvBuf(Size size);

		/* ioctl wrappers */
		bool getIfConf(struct ifconf &cfg);
		bool getV4IfAddr(const String &ifname, InAddress &addr);
		bool getV6IfAddr(const String &ifname, InAddress &addr);
		bool getIfBcastAddr(const String &ifname, struct sockaddr &addr);
		bool addV4IfAddr(const InetIfAliasReq &req, int idx = -1);
		bool addV6IfAddr(const Inet6IfAliasReq &req, int idx = -1);
		bool delV4IfAddr(const InetIfAliasReq &req, int idx = -1);
		bool delV6IfAddr(const Inet6IfAliasReq &req);

		bool configure(const SockOpt &opt);

		int fd() const { return theFD; }
		bool isOpen() const { return theFD >= 0; }
		bool isBlocking() const;
		int flags() const;
		int lport() const;
		int rport() const;
		Time rcvTimeout() const;
		Size sendBuf() const;
		Size recvBuf() const;
		Error error() const;
		bool peerAddr(struct sockaddr *addr, socklen_t &len) const;
		NetAddr peerAddr() const;
		NetAddr localAddr() const;

		operator void*() const { return theFD >= 0 ? (void*)-1 : (void*)0; }

		// always treat two these methods as protected!
		bool setOpt(int level, int optName, const void *optVal, socklen_t optLen);
		bool getOpt(int level, int optName, void *optVal, socklen_t *optLen) const;

	protected:
		bool noteNewFD();

		bool setOpt(int optName, bool set);
		bool setOpt(int optName, const void *optVal, socklen_t optLen);
		bool getOpt(int optName, void *optVal, socklen_t *optLen) const;

		Size sysRead(void *buf, Size sz);
		Size sysWrite(const void *buf, Size sz);
		bool sysClose();

		bool numberedIface(InetIfAliasReq &r, int idx) const;
		bool unsupOpt(void *dummy = 0) const;

	protected:
		int theFD;

		static int TheLevel; // number of open sockets
};

#endif
