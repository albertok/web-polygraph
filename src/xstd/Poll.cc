
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <limits.h>
#include "xstd/h/os_std.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Assert.h"
#include "xstd/Clock.h"
#include "xstd/Poll.h"

#include "xstd/gadgets.h"

#include <limits>

PollFD::PollFD(int aFd) {
	init(aFd);
}

void PollFD::init(int aFd) {
	this->fd = aFd;
	clear(dirNone);
}

void PollFD::set(IODir dir) {
	if (dir == dirRead)
		events |= XPOLL_RD;
	else
		events |= XPOLL_WR;
}

void PollFD::clear(IODir dir) {
	if (dir == dirRead) {
		events &= ~XPOLL_RD;
		revents &= ~XPOLL_RD;
	} else
	if (dir == dirWrite) {
		events &= ~XPOLL_WR;
		revents &= ~XPOLL_WR;
	} else
		events = 0;

	if (!events)
		revents = 0;
}

PollFD PollFD::unidir(IODir dir) const {
	PollFD res = *this;
	res.clear((IODir)-dir);
	res.set(dir);
	return res;
}

/* Poll */

void Poll::configure() {
	FileScanner::configure(numeric_limits<int>::max());
	thePollees.resize(fdLimit());
	theHotFDs.stretch(fdLimit());
	theHotIdx.stretch(fdLimit());

	for (int fd = 0; fd < thePollees.count(); ++fd) {
		thePollees[fd].init(fd);
		theHotIdx[fd] = -1;
	}
}

FileScanReserv Poll::setFD(int fd, IODir dir, FileScanUser *u) {
	Assert(0 <= fd && fd < thePollees.count());
	thePollees[fd].set(dir);
	if (fd > theMaxFD)
		theMaxFD = fd;
	return FileScanner::setFD(fd, dir, u);
}

void Poll::clearFD(int fd, IODir dir) {
	Assert(0 <= fd && fd < thePollees.count());
	thePollees[fd].clear(dir);
	if (theHotIdx[fd] >= 0)
		theHotFDs[theHotIdx[fd]].clear(dir);

	if (fd == theMaxFD) {
		while (theMaxFD >= 0 && !thePollees[theMaxFD].used()) {
			theMaxFD--;
		}
	}

	FileScanner::clearFD(fd, dir);
}

FileScanUser *Poll::readyUser(int idx, IODir dir, int &fd) {
	const PollFD &pfd = theHotFDs[idx];
	fd = pfd.fd;

	// quit if fd is bad
	if (fd < 0) {
		static int bugCount = 0;
		if (bugCount++ % 1000 == 0)
			clog << here << "bug: invalid fd `" << fd 
				<< "' returned by poll(2); #" << bugCount << endl;
		return 0;
	}

	// quit if fd is stale
	if (fd > theMaxFD)
		return 0;

	// quit if poll(2) said "not ready"
	if (!pfd.ready(dir))
		return 0;

	// Quit if not waiting for this event and poll(2) returns XPOLL_EX.
	// Will likely be handled by another direction. Or this is a
	// different user and event must be ignored.
	if (!thePollees[fd].interested(dir) && theRegs[fd].blocked() != dir)
		return 0;

	return theRegs[fd].user();
}

int Poll::sweep(Time *timeout) {
	theHotFDs.reset();
	// copy active fds to hot array
	for (int fd = 0; fd <= theMaxFD; ++fd) {
		const PollFD &p = thePollees[fd];
		const FileScanUserReg &reg = theRegs[fd];
		if (p.used() && reg.needsCheck()) {
			theHotIdx[fd] = theHotFDs.count();
			if (const IODir dir = (IODir)reg.blocked())
				theHotFDs.append(p.unidir(dir));
			else
				theHotFDs.append(p);
		} else
			theHotIdx[fd] = -1;
	}

	if (!theHotFDs.count())
		return 0; // XXX: should not we sleep for timeout anyway?

	const int res = xpoll(theHotFDs.items(), theHotFDs.count(),
		timeout ? timeout->msec() : -1);

	return res <= 0 ? res : theHotFDs.count();
}
