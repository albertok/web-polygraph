
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iomanip.h"
#include "xstd/h/iostream.h"

#include <limits>

#include "xstd/Epoll.h"

EpollEventMask::EpollEventMask(const IODir dir):
	theMask(0), theClearedSweepId(0) {
	if (dir)
		set(dir);
}

EpollEvent::EpollEvent(const int aFd, const EpollEventMask ev) {
	memset(&data, 0, sizeof(data));
	data.fd = aFd;
	events = ev.value();
}

/* Epoll */

Epoll::Epoll(): theFD(-1) {
}

void Epoll::configure() {
	FileScanner::configure(numeric_limits<int>::max());

	theFD = xepoll_create(fdLimit());
	Must(theFD >= 0);

	theHotties.resize(fdLimit());
	theInterests.resize(fdLimit());

	theEventsCount = -1;
	theLastSweepId = 0;
}

FileScanReserv Epoll::setFD(int fd, IODir dir, FileScanUser *u) {
	Assert(0 <= fd && fd < theHotties.count());
	if (fd > theMaxFD)
		theMaxFD = fd;

	const int op = theInterests[fd].used() ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
	theInterests[fd].set(dir);
	EpollEventMask ev;
	if (theRegs[fd].blocked()) {
		// there are blocked events
		if (op == EPOLL_CTL_ADD)
			// fd is added, register blocked events instead
			ev.set((IODir)theRegs[fd].blocked());
		// else fd is already registered, do not change it until it is unblocked
	} else
		// no blocked events, set events to what we are asked for
		ev = theInterests[fd];
	if (ev.used()) {
		EpollEvent event(fd, ev);
		Must(xepoll_ctl(theFD, op, fd, &event) == 0);
	}

	return FileScanner::setFD(fd, dir, u);
}

void Epoll::clearFD(int fd, IODir dir) {
	Assert(0 <= fd && fd < theHotties.count());

	if (theInterests[fd].used()) {
		theInterests[fd].clear(dir);

		if (!theInterests[fd].used())
			theInterests[fd].clearedSweepId(theLastSweepId);

		const int op = theInterests[fd].used() ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
		if (!theRegs[fd].blocked() || op == EPOLL_CTL_DEL) {
			EpollEvent event(fd, theInterests[fd]);
			Must(xepoll_ctl(theFD, op, fd, &event) == 0);

			if (fd == theMaxFD && op == EPOLL_CTL_DEL)
				while (theMaxFD >= 0 && !theInterests[theMaxFD].used())
					--theMaxFD;
		}
	}

	FileScanner::clearFD(fd, dir);
}

FileScanUser *Epoll::readyUser(int idx, IODir dir, int &fd) {
	Assert(0 <= idx && idx < theEventsCount);
	const EpollEvent &pfd = theHotties[idx];
	fd = pfd.data.fd;

	// quit if fd is bad
	if (fd < 0) {
		static int bugCount = 0;
		if (bugCount++ % 1000 == 0)
			clog << here << "bug: invalid fd `" << fd 
				<< "' returned by epoll_wait(2); #" << bugCount << endl;
		return 0;
	}

	// quit if fd is stale
	if (fd > theMaxFD)
		return 0;

	// quit if fd was cleared during this sweep iteration
	if (theInterests[fd].clearedSweepId() == theLastSweepId)
		return 0;

	// quit if epoll_wait(2) said "not ready"
	if (!pfd.ready(dir))
		return 0;

	// Quit if not waiting for this event and epoll_wait(2) returns
	// XEPOLL_EX. Will likely be handled by another direction. Or
	// this is a different user and event must be ignored.
	if (!interested(fd, dir) && theRegs[fd].blocked() != dir)
		return 0;

	// if fd was blocked, unblock it now
	// theRegs[fd] will be unblocked in FileScanUserReg::notifyRead/WriteReady()
	// but we must maintain epoll-specific state here
	if (theRegs[fd].blocked() == dir) {
		Assert(theInterests[fd].used());
		EpollEvent event(fd, theInterests[fd]);
		Must(xepoll_ctl(theFD, EPOLL_CTL_MOD, fd, &event) == 0);
	}

	return theRegs[fd].user();
}

int Epoll::sweep(Time *timeout) {
	++theLastSweepId;
	if (!theLastSweepId)
		++theLastSweepId;

	return theEventsCount = xepoll_wait(theFD, theHotties.items(), theHotties.count(),
		timeout ? timeout->msec() : -1);
}

void Epoll::setReadNeedsWrite(int fd) {
	FileScanner::setReadNeedsWrite(fd);
	setBlocked(fd);
}

void Epoll::setWriteNeedsRead(int fd) {
	FileScanner::setWriteNeedsRead(fd);
	setBlocked(fd);
}

void Epoll::setBlocked(const int fd) {
	Assert(0 <= fd && fd < theHotties.count());
	if (theInterests[fd].used()) {
		EpollEvent event(fd, (IODir)theRegs[fd].blocked());
		Must(xepoll_ctl(theFD, EPOLL_CTL_MOD, fd, &event) == 0);
	}
}

// Check the requested events because returned events may contain ERROR
// instead of READ or WRITE flag.
bool Epoll::interested(const int fd, const IODir dir) const {
	Assert(0 <= fd && fd < theHotties.count());
	const unsigned long event(theInterests[fd].value());
	return (dir == dirRead ? 
		(event & XEPOLL_RD) : (event & XEPOLL_WR)) != 0;
}
