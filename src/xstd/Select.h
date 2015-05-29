
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_SELECT_H
#define POLYGRAPH__XSTD_SELECT_H

#include "xstd/Array.h"
#include "xstd/FileScanner.h"

// Select is a File Scanner based on select(2) system call


// a wrapper arround fd_set
class FD_Set {
	typedef FileScanUser User;
	public:
		FD_Set();

		int maxFD() const { return theMaxFD; }
		int resCount() const { return theResCount; }
		bool active() const { return theMaxFD >= 0; }
		fd_set &prepReadySet() { theReadySet = theSet; return theReadySet; }

		void setFD(int fd, User *u);
		void clearFD(int fd);

		bool isSet(int fd) const { return FD_ISSET(fd, &theSet) != 0; }
		bool isReady(int fd) const { return FD_ISSET(fd, &theReadySet) != 0; }

	protected:
		fd_set theSet;
		fd_set theReadySet;

		int theMaxFD;
		int theResCount;
};


class Select: public FileScanner {
	public:
		typedef FileScanUser User;

	public:
		virtual const char *name() const { return "select"; }

		virtual void configure();

		virtual FileScanReserv setFD(int fd, IODir dir, User *p);
		virtual void clearFD(int fd, IODir dir);

	protected:
		virtual int sweep(Time *timeout = 0);
		virtual User *readyUser(int idx, IODir dir, int &fd);
		void prepReadySets(fd_set *&rdSet, fd_set *&wrSet);
		FD_Set &fdSet(const IODir dir);
		const FD_Set &fdSet(const IODir dir) const;
		bool active() const;

	protected:
		FD_Set theSets[2]; // read/write
};

#endif
