
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_EPOLL_H
#define POLYGRAPH__XSTD_EPOLL_H

#include "xstd/h/epoll.h"
#include "xstd/h/stdint.h"
#include "xstd/FileScanner.h"

#include <limits>

// Epoll is a File Scanner based on epoll(7)

// aggregate bit masks together (or not)
#define XEPOLL_RD EPOLLIN
#define XEPOLL_WR EPOLLOUT
#define XEPOLL_EX (EPOLLERR | EPOLLHUP)

class EpollEventMask {
	public:
		EpollEventMask(const IODir dir = dirNone);
		inline void set(const IODir dir);
		inline void clear(const IODir dir);
		inline void clearedSweepId(const uint64_t aClearedSweepId);
		inline uint64_t clearedSweepId() const;
		inline unsigned long value() const;
		inline bool used() const;

	private:
		unsigned long theMask;
		uint64_t theClearedSweepId;
};

// epoll_event wrapper
// note: optimized for efficient copying and such
class EpollEvent: public epoll_event {
	public:
		EpollEvent(const int aFD = -1, const EpollEventMask ev = EpollEventMask());

		inline bool ready(const IODir dir) const;
};

class Epoll: public FileScanner {
	public:
		typedef FileScanUser User;

	public:
		Epoll();

		virtual const char *name() const { return "epoll"; }

		virtual void configure();

		virtual FileScanReserv setFD(int fd, IODir dir, User *p);
		virtual void clearFD(int fd, IODir dir);

		virtual void setReadNeedsWrite(int fd);
		virtual void setWriteNeedsRead(int fd);

	protected:
		virtual int sweep(Time *timeout = 0);
		virtual User *readyUser(int idx, IODir dir, int &fd);
		void setBlocked(const int fd);
		bool interested(const int fd, const IODir dir) const;

	protected:
		Array<EpollEvent> theHotties; // used to get results from epoll_wait
		Array<EpollEventMask> theInterests;
		int theFD;
		int theEventsCount;
		uint64_t theLastSweepId;
};


/* inlined methods */

inline
void EpollEventMask::set(const IODir dir) {
	theMask |= dir == dirRead ? XEPOLL_RD : XEPOLL_WR;
}

inline
void EpollEventMask::clear(const IODir dir) {
	switch (dir) {
		case dirRead:
			theMask &= ~XEPOLL_RD;
			break;
		case dirWrite:
			theMask &= ~XEPOLL_WR;
			break;
		default:
			theMask = 0;
	}
}

inline
void EpollEventMask::clearedSweepId(const uint64_t aClearedSweepId) {
	theClearedSweepId = aClearedSweepId;
}

inline
uint64_t EpollEventMask::clearedSweepId() const {
	return theClearedSweepId;
}

inline
unsigned long EpollEventMask::value() const {
	return theMask;
}

inline
bool EpollEventMask::used() const {
	return theMask != 0;
}

inline
bool EpollEvent::ready(const IODir dir) const {
	return (dir == dirRead ?
		(events & (XEPOLL_RD | XEPOLL_EX)): 
		(events & (XEPOLL_WR | XEPOLL_EX))) != 0;
}

#endif
