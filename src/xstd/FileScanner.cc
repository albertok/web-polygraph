
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Assert.h"
#include "xstd/Clock.h"
#include "xstd/FileScanner.h"

#include "xstd/gadgets.h"

FileScanUserPriority FileScanner::TheMinPriority = fsupMin;


/* FileScanUser */

FileScanUser::~FileScanUser() {
}

// A user may not want to implement read or write
// The default implementation will barf in case the
// corresponding method is actually called
void FileScanUser::noteReadReady(int) {
	Assert(false);
}

void FileScanUser::noteWriteReady(int) {
	Assert(false);
}

void FileScanUser::noteTimeout(int, Time) {
	Assert(false);
}


/* FileScanReserv */

FileScanReserv::FileScanReserv(): theFD(-1), theDir(dirNone), isReady(false) {
}

FileScanReserv::FileScanReserv(int aFD, IODir aDir): theFD(aFD), theDir(aDir), isReady(false) {
}


/* FileScanUserReg */

// note: any activity on the Reg record should reset theStart member
// to upgrade timeout expiration

void FileScanUserReg::reset() {
	theUser = 0;
	theStart = theTimeout = Time();
	theResCount = 0;
	thePriority = fsupDefault;
	theBlockingDir = dirNone;
	isForcedReady = false;
}

void FileScanUserReg::set(FileScanUser *aUser, IODir dir) {
	Assert(aUser);
	if (theUser) {
		Assert(theUser == aUser);
		Assert(theResCount == 1);
	} else {
		theUser = aUser;
		if (theBlockingDir == dir) // so that we do not steal notifications
			theBlockingDir = dirNone;
	}
	thePriority = fsupDefault;
	theStart = TheClock;
	theResCount++;
}

void FileScanUserReg::clear(IODir dir) {
	Assert(theUser);
	Assert(theResCount > 0);
	if (--theResCount == 0) {
		reset();
	} else {
		if (dir == dirRead)
			isForcedReady = false;
		if (dir == -theBlockingDir)  // the blocked direction was cleared
			theBlockingDir = dirNone;
		theStart = TheClock;
	}
}

void FileScanUserReg::changeUser(FileScanUser *uOld, FileScanUser *uNew) {
	Assert(uOld && uNew);

	// be strict and disallow changes for multi reservations
	// though this code does not care
	if (!Should(theResCount == 1))
		return;

	if (!Should(theUser == uOld))
		return;

	theUser = uNew;
}

void FileScanUserReg::timeout(Time aTout) {
	theTimeout = aTout;
}

void FileScanUserReg::priority(Priority aPriority) {
	thePriority = aPriority;
}

Time FileScanUserReg::waitTime() const {
	return TheClock - theStart;
}

bool FileScanUserReg::timedout() const {
	return thePriority >= FileScanner::TheMinPriority &&
		theTimeout >= 0 && theTimeout <= waitTime();
}

bool FileScanUserReg::needsCheck() const {
	return theUser && thePriority >= FileScanner::TheMinPriority;
}

void FileScanUserReg::noteIo() {
	theStart = TheClock;
	thePriority = fsupDefault;
}

void FileScanUserReg::notifyReadReady(int fd) {
	// note: it is unsafe to do anything after notifying the user
	if (Should(theUser)) {
		noteIo();
		isForcedReady = false;
		if (theBlockingDir == dirRead) {
			theBlockingDir = dirNone;
			theUser->noteWriteReady(fd); // write was blocked on read
		} else {
			// we should not be doing direct reads if we are blocked on write
			Should(theBlockingDir == dirNone);
			theUser->noteReadReady(fd);
		}
	}
}

void FileScanUserReg::notifyWriteReady(int fd) {
	// note: it is unsafe to do anything after notifying the user
	if (Should(theUser)) {
		noteIo();
		if (theBlockingDir == dirWrite) {
			theBlockingDir = dirNone;
			theUser->noteReadReady(fd); // read was blocked by write
		} else {
			// we should not be doing direct writes if we are blocked on read
			Should(theBlockingDir == dirNone);
			theUser->noteWriteReady(fd);
		}
	}
}

void FileScanUserReg::blockedOn(IODir dir) {
	Should(!isForcedReady); // sanity/simplicity check
	if (Should(theBlockingDir != -dir)) // no deadlocks
		theBlockingDir =dir;
}

int FileScanUserReg::blocked() const {
	return theBlockingDir; // assumes that dirNone is zero/false
}

void FileScanUserReg::forcedReady(bool be) {
	if (theUser)
		isForcedReady = be;
}

FileScanUser *FileScanUserReg::forcedReady() const {
	return theUser && isForcedReady ? theUser : 0;
}


/* FileScanner */

FileScanner::FileScanner(): theMaxFD(-1), theResCount(0), theTicker(0) {
}

FileScanner::~FileScanner() {
}

void FileScanner::configure(int fdLimit) {
	const int rlimit = SetMaxFD(GetMaxFD());
	fdLimit = Min(fdLimit, rlimit);
	if (fdLimit <= 0)
		cerr << "error: zero filedescriptor limit?!" << endl << xexit;

	theRegs.resize(fdLimit);
	for (int fd = 0; fd < fdLimit; ++fd)
		theRegs[fd].reset();
	theForcedReady.stretch(fdLimit);
}

void FileScanner::ticker(FileScanTicker *aTicker) {
	Assert(!theTicker);
	theTicker = aTicker;
}

void FileScanner::checkTimeouts() {
	for (int fd = 0; fd <= theMaxFD; ++fd) {
		FileScanUserReg &reg = theRegs[fd];
		if (reg.timedout()) {
			if (User *u = reg.user())
				u->noteTimeout(fd, reg.waitTime());
		}
	}
}

int FileScanner::scan(Priority minP, Time *timeout) {

	if (timeout && *timeout < 0) // no time to sweep
		return 0;

	// forced-ready users should "continue" their buffered reads
	// as if they were able to read everything in one call
	if (const int forcedReady = scanForced())
		return forcedReady;

	TheMinPriority = minP;

	// silent fix:
	// unfortunately, some selects(2) have magic limits on timeouts
	if (timeout && timeout->tv_sec > 100000000)
		timeout->tv_sec = 100000000;

	//cerr << here << "reserv: " << theResCount << endl;
	const int hotCount = sweep(timeout);
	//cerr << here << "hot files: " << hotCount << endl;

	if (theTicker)
		theTicker->begCycle(hotCount);

	for (int idx = 0; idx < hotCount; ++idx) {
		int fd;
		bool active = false;
		//cerr << here << "idx: " << idx << " ready: " << readyUser(idx, dirRead, fd) << " & " << readyUser(idx, dirWrite, fd) << endl;
		if (readyUser(idx, dirRead, fd)) {
			FileScanUserReg &reg = theRegs[fd];
			reg.notifyReadReady(fd);
			active = true;
		}
		if (readyUser(idx, dirWrite, fd)) {
			FileScanUserReg &reg = theRegs[fd];
			reg.notifyWriteReady(fd);
			active = true;
		}

		if (active && theTicker && !theTicker->tick())
			break;
	}

	checkTimeouts();

	if (theTicker)
		theTicker->endCycle();

	return hotCount;
}

int FileScanner::scanForced() {
	// should we use theTicker here?
	int hotCount = 0;
	while (theForcedReady.count()) {
		const int fd = theForcedReady.pop();
		FileScanUserReg &reg = theRegs[fd];
		if (reg.forcedReady()) {
			reg.notifyReadReady(fd);
			hotCount++;
		}
	}
	return hotCount;
}

FileScanReserv FileScanner::setFD(int fd, IODir dir, FileScanUser *u) {
	theResCount++;
	theRegs[fd].set(u, dir);
	return FileScanReserv(fd, dir);
}

void FileScanner::setTimeout(int fd, Time tout) {
	Assert(fd >= 0 && fd <= theMaxFD);
	theRegs[fd].timeout(tout);
}

void FileScanner::clearTimeout(int fd) {
	theRegs[fd].timeout(Time());
}

void FileScanner::setPriority(int fd, Priority p) {
	theRegs[fd].priority(p);
}

FileScanner::Priority FileScanner::getPriority(int fd) const {
	return theRegs[fd].priority();
}

void FileScanner::clearFD(int fd, IODir dir) {
	theRegs[fd].clear(dir);
	theResCount--;
}

bool FileScanner::user(const FileScanReserv &res, const FileScanUser *u) const {
	return res && theRegs[res.fd()].user(u);
}

void FileScanner::changeUser(const FileScanReserv &res, FileScanUser *uOld, FileScanUser *uNew) {
	theRegs[res.fd()].changeUser(uOld, uNew);
}

void FileScanner::setReadNeedsWrite(int fd) {
	theRegs[fd].blockedOn(dirWrite);
}

void FileScanner::setWriteNeedsRead(int fd) {
	theRegs[fd].blockedOn(dirRead);
}

void FileScanner::forceReady(int fd) {
	theRegs[fd].forcedReady(true);
}
