
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/os_std.h"

#include "xstd/Assert.h"
#include "xstd/Select.h"
#include "xstd/gadgets.h"


FD_Set::FD_Set(): theMaxFD(-1), theResCount(0) {
	FD_ZERO(&theSet);
	FD_ZERO(&theReadySet);
}

void FD_Set::setFD(int fd, FileScanUser *u) {
	Assert(fd >= 0 && u);

	FD_SET(fd, &theSet);
	if (fd > theMaxFD)
		theMaxFD = fd;
	theResCount++;
}

void FD_Set::clearFD(int fd) {
	Assert(fd >= 0);
	Assert(fd <= theMaxFD);

	FD_CLR(fd, &theSet);
	FD_CLR(fd, &theReadySet); // poll() cannot do that

	if (fd == theMaxFD) {
		while (theMaxFD >= 0 && !FD_ISSET(theMaxFD, &theSet)) {
			theMaxFD--;
		}
	}

	theResCount--;
}


/* Select */

void Select::configure() {
	int rlimit = GetMaxFD();
	if (rlimit > static_cast<int>(FD_SETSIZE)) {
		cerr << "warning: getrlimit(2) system call reports " <<
			rlimit << " file descriptors while FD_SETSIZE "
			"#defines only " << FD_SETSIZE <<
			"; using lower value." << endl;
		rlimit = FD_SETSIZE;
	}

	FileScanner::configure(rlimit);
}

FileScanReserv Select::setFD(int fd, IODir dir, FileScanUser *u) {
	fdSet(dir).setFD(fd, u);
	if (fd > theMaxFD)
		theMaxFD = fd;
	return FileScanner::setFD(fd, dir, u);
}

void Select::clearFD(int fd, IODir dir) {
	fdSet(dir).clearFD(fd);
	if (fd == theMaxFD)
		theMaxFD = Max(fdSet(dirRead).maxFD(), fdSet(dirWrite).maxFD());
	FileScanner::clearFD(fd, dir);
}

FileScanUser *Select::readyUser(int idx, IODir dir, int &fd) {
	fd = idx;
	return fdSet(dir).isReady(fd) ? theRegs[fd].user() : 0;
}

void Select::prepReadySets(fd_set *&rdSet, fd_set *&wrSet) {
	if (!active()) {
		rdSet = 0;
		wrSet = 0;
		return;
	}

	bool rdSetActive = false;
	bool wrSetActive = false;
	rdSet = &fdSet(dirRead).prepReadySet();
	wrSet = &fdSet(dirWrite).prepReadySet();
	for (int fd = 0; fd <= theMaxFD; ++fd) {
		if (fdSet(dirRead).isSet(fd)) {
			if (theRegs[fd].blocked() == dirWrite) {
				FD_SET(fd, wrSet);
				FD_CLR(fd, rdSet);
				wrSetActive = true;
			} else
				rdSetActive = true;
		} else if (fdSet(dirWrite).isSet(fd)) {
			if (theRegs[fd].blocked() == dirRead) {
				FD_SET(fd, rdSet);
				FD_CLR(fd, wrSet);
				rdSetActive = true;
			} else
				wrSetActive = true;
		}
	}

	if (!rdSetActive)
		rdSet = 0;
	if (!wrSetActive)
		wrSet = 0;
}

int Select::sweep(Time *timeout) {
	// have to check all fds, cannot choose like poll() does
	const int hotCount = theMaxFD+1;

	fd_set *rdSet = 0;
	fd_set *wrSet = 0;
	fd_set *excSet = 0;
	prepReadySets(rdSet, wrSet);

#	if defined(WIN32)
		// MS Windows reports connect(2) failures via exception set
		// collect exceptions and later "backport" to wrSet to fake
		// Unix-like behavior
		if (wrSet) {
			static fd_set es;
			es = *wrSet;
			excSet = &es;
		} else
		if (!rdSet) {
			// Windows select(2) fails if no FDs are checked for
			// we must emulate sleep instead of calling select(2)
			if (timeout)
				sleep(timeout->sec());
			return 0;
		}
#	endif

	const int res = ::select(hotCount, rdSet, wrSet, excSet, timeout);

#	if defined(WIN32)
		if (res > 0 && excSet && wrSet) {
			for (int fd = 0; fd < hotCount; ++fd) {
				if (FD_ISSET(fd, excSet))
					FD_SET(fd, wrSet);
			}
		}
#	endif

	return res <= 0 ? res : hotCount;
}

FD_Set &Select::fdSet(const IODir dir) {
	Assert(dir != dirNone);
	return dir == dirRead ? theSets[0] : theSets[1];
}

const FD_Set &Select::fdSet(const IODir dir) const {
	Assert(dir != dirNone);
	return dir == dirRead ? theSets[0] : theSets[1];
}

bool Select::active() const {
	return fdSet(dirRead).active() || fdSet(dirWrite).active();
}
