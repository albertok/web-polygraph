
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_POLL_H
#define POLYGRAPH__XSTD_POLL_H

#include "xstd/h/poll.h"
#include "xstd/FileScanner.h"

// Poll is a File Scanner based on poll(2) system call

// aggregate bit masks together (or not)
#define XPOLL_RD POLLRDNORM
#define XPOLL_WR POLLWRNORM
#define XPOLL_EX (POLLERR | POLLHUP | POLLNVAL)

// pollfd wrapper
// note: optimized for efficient copying and such
class PollFD: public pollfd {
	public:
		PollFD(int aFD = -1);

		void init(int aFD); // must be called to initialize the object

		bool used() const { return events != 0; }
		inline bool interested(IODir dir) const;
		inline bool ready(IODir dir) const;
		
		void set(IODir dir);
		void clear(IODir dir);

		PollFD unidir(IODir dir) const;
};

class Poll: public FileScanner {
	public:
		typedef FileScanUser User;

	public:
		virtual const char *name() const { return "poll"; }

		virtual void configure();

		virtual FileScanReserv setFD(int fd, IODir dir, User *p);
		virtual void clearFD(int fd, IODir dir);

	protected:
		virtual int sweep(Time *timeout = 0);
		virtual User *readyUser(int idx, IODir dir, int &fd);

	protected:
		Array<PollFD> thePollees;

	private:
		Array<PollFD> theHotFDs; // used during poll call and cleared in clearFD()
		Array<int> theHotIdx; // FD : position in theHotFDs map, used for clearing theHotFDs
};


/* inlined methods */

inline
bool PollFD::interested(IODir dir) const {
	return (dir == dirRead ? 
		(events & XPOLL_RD) : (events & XPOLL_WR)) != 0;
}

inline
bool PollFD::ready(IODir dir) const {
	return (dir == dirRead ?
		(revents & (XPOLL_RD | XPOLL_EX)): 
		(revents & (XPOLL_WR | XPOLL_EX))) != 0;
}

#endif
