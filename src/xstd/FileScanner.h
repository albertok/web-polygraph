
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_FILESCANNER_H
#define POLYGRAPH__XSTD_FILESCANNER_H

#include "xstd/h/sys/types.h"
#include "xstd/h/os_std.h"

#include "xstd/Array.h"
#include "xstd/Time.h"

// file scanner is a common interface to select(2), poll(2) and epoll(7)

typedef enum { dirNone = 0, dirRead = -1, dirWrite = +1 } IODir;

class FileScanUser {
	public:
		typedef void (FileScanUser::*NotifMethod)(int fd);

	public:
		virtual ~FileScanUser();

		// default implementation will just abort()
		virtual void noteReadReady(int fd);
		virtual void noteWriteReady(int fd);
		virtual void noteTimeout(int fd, Time tout);
};

// reservation receipt
class FileScanReserv {
	public:
		FileScanReserv();
		FileScanReserv(int aFD, IODir aDir);

		operator bool() const { return reserved(); }
		bool reserved() const { return theFD >= 0; }
		bool ready() const { return isReady; }
		int fd() const { return theFD; }
		IODir dir() const { return theDir; }

		void ready(bool be) { isReady = be; }
		void clear() { theFD = -1; }

	protected:
		int theFD;
		IODir theDir;
		bool isReady;
};

// Ticker is notified about various stages of a scan process
class FileScanTicker {
	public:
		virtual ~FileScanTicker() {}

		virtual void begCycle(int readyCount) = 0;
		virtual void endCycle() = 0;
		virtual bool tick() = 0;
};

typedef enum { fsupMin, fsupBestEffort, fsupAsap, fsupDefault } FileScanUserPriority;

// an internal registration record for a user to monitor I/O activity
class FileScanUserReg {
	public:
		typedef FileScanUserPriority Priority;
		typedef FileScanUser User;

	public:
		FileScanUserReg() {}

		void reset();

		void set(FileScanUser *aUser, IODir dir); // updates start time, res++
		void clear(IODir dir); // decrements reservation counter
		void timeout(Time aTout);
		void priority(Priority aPriority);
		void blockedOn(IODir dir);
		void forcedReady(bool be);

		FileScanUser *user() { return theUser; }
		bool user(const FileScanUser *u) const { return theUser == u && u; }
		void changeUser(FileScanUser *uOld, FileScanUser *uNew);

		Priority priority() const { return thePriority; }
		Time waitTime() const;

		bool timedout() const;
		bool needsCheck() const;
		int blocked() const; // returns IODir (dirNone == false)
		FileScanUser *forcedReady() const;

		void notifyReadReady(int fd);
		void notifyWriteReady(int fd);

	protected:
		void noteIo();

	protected:
		User *theUser;
		Time theTimeout;      // how long a user is willing to wait for io
		Time theStart;        // registration or last io
		int theResCount;      // how many reservations (e.g., read + write)
		Priority thePriority;
		IODir theBlockingDir; // set if read/write is blocked on write/read
		bool isForcedReady;
};

// the "scanner" itself
class FileScanner {
	friend class FileScanUserReg;

	public:
		typedef FileScanUser User;
		typedef FileScanTicker Ticker;
		typedef FileScanUserPriority Priority;

	protected:
		static Priority TheMinPriority;  // min priority to be scan()ned

	public:
		FileScanner();
		virtual ~FileScanner();

		// scanner's name (e.g., system call for syscall wrappers)
		virtual const char *name() const = 0;

		virtual void configure() = 0;
		void ticker(Ticker *aTicker);
		int fdLimit() const { return theRegs.count(); }

		bool idle() const { return theResCount <= 0; }

		virtual FileScanReserv setFD(int fd, IODir dir, User *p);
		virtual void clearFD(int fd, IODir dir);
		void clearRes(FileScanReserv &res) { clearFD(res.fd(), res.dir()); res.clear(); }
		bool user(const FileScanReserv &res, const FileScanUser *u) const;
		void changeUser(const FileScanReserv &res, FileScanUser *uOld, FileScanUser *uNew);

		// the calls below are needed to handle OpenSSL I/O API
		virtual void setReadNeedsWrite(int fd); // call noteReadReady if write is ready
		virtual void setWriteNeedsRead(int fd); // call noteWriteReady if read is ready
		void forceReady(int fd); // call noteReadReady w/o checking fd is ready

		void setTimeout(int fd, Time tout);
		void clearTimeout(int fd);

		void setPriority(int fd, Priority p);
		Priority getPriority(int fd) const;

		// returns a positive number if some files where ready; -1 on error
		// calls applicable User methods for ready files
		// null timeout means wait until error or at least one file is ready
		int scan(Priority minP, Time *timeout = 0);
		int scan(Time *tout = 0) { return scan(fsupMin, tout); }

	protected:
		void configure(int fdLimit);

		// return "hot count" (see scan code)
		virtual int sweep(Time *timeout = 0) = 0;
		int scanForced();

		// must set fd if the indexed user is ready
		virtual User *readyUser(int idx, IODir dir, int &fd) = 0;

		void checkTimeouts();

	protected:
		Array<FileScanUserReg> theRegs; // user registrations
		Array<int> theForcedReady;      // fd of users that have buffered reads

		int theMaxFD;
		int theResCount;
		Ticker *theTicker;
};

// we must add an iterator to make the interface complete

#endif
