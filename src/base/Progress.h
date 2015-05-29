
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_PROGRESS_H
#define POLYGRAPH__BASE_PROGRESS_H

#include "xstd/Time.h"

class OLog;
class ILog;

// global progress record for a given protocol
class ProtoProgress {
	public:
		ProtoProgress(const char *anId, const char *aName): id(anId),
			name(aName), successes(0) {}

	public:
		const char *id; // for object IDs and labels; no deletion
		const char *name; // for human-facing reports: no deletion
		int successes; // successful transactions
};

// global progress record, updated by stat cycles
class Progress {
	public:
		Progress();

		int xacts() const { return theXactCnt; }
		int errs() const { return theErrCnt; }
		Time time() const;

		void success() { theXactCnt++; }
		void failure() { theErrCnt++; }

		void store(OLog &ol) const;
		void load(ILog &il);
		ostream &print(ostream &os) const;

	public:
		ProtoProgress socks;
		ProtoProgress ssl;
		ProtoProgress ftp;

	protected:
		Time theTimestamp; // place to keep Clock after load()
		int theXactCnt;    // all successful completions
		int theErrCnt;     // all failed transactions
};

extern Progress TheProgress;

#endif
