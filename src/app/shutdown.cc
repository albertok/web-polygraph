
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/new.h"
#include "xstd/Socket.h"
#include "xstd/String.h"
#include "xstd/AlarmClock.h"
#include "runtime/LogComment.h"
#include "runtime/PolyOLog.h"
#include "base/Progress.h"
#include "runtime/SharedOpts.h"
#include "runtime/StatPhaseMgr.h"
#include "runtime/globals.h"
#include "app/shutdown.h"


static String TheReason = 0;
static int TheSignal = -1;

static bool DoShutdown = false; // ShutdownReason() has been called
static bool InShutdown = false; // Shutdown() has started, has not finished
static bool IsClean = false;    // Shutdown() has finished

// extra memory to use when we are running out of memory
static char *TheMemReserve = new char[16*1024*1024];


static void Shutdown();


// check various shutdown conditions
bool ShutdownNow() {
	if (DoShutdown)
		return true; // already decided
	else
	if (Socket::Level() >= Socket::TheMaxLevel) {
		Comment << "active file descriptors: " << Socket::Level() << "; limit: " << Socket::TheMaxLevel << endc;
		ShutdownReason("dangerously low on file descriptors");
	} else
	if (TheSignal >= 0) {
		Comment << "noticed shutdown signal (" << TheSignal << ")" << endc;
		ShutdownReason("got shutdown signal");
	} else
		return false;

	return DoShutdown = true;
}

bool ShutdownRequested() {
	return DoShutdown;
}

void ShutdownSignal(int s) {
	TheSignal = s;
	clog << "got shutdown signal (" << s << ")" << endl;
}

void ShutdownReason(const String &reason) {
	if (reason && !TheReason) // do not overwrite first reason
		TheReason = reason;
	DoShutdown = true;
}

void ShutdownAtExit() {
	ShutdownReason("fatal error");
	Shutdown();
}

void ShutdownAtNew() {
	delete[] TheMemReserve; TheMemReserve = 0;

	try {
		Should(xset_new_handler(0));
		clog << "ran out of memory" << endl;
		ShutdownReason("insufficient memory");
		Shutdown();
	} catch (...) {
		clog << "out of memory handler failed" << endl;
	}

	// must terminate the program
	exit(-1);
}

static
void Shutdown() {
	if (IsClean || InShutdown)
		return; // nothing to do; probably was here already

	DoShutdown = InShutdown = true;

	Clock::Update(false);

	Comment(2) << "got " << TheProgress.xacts()
		<< " xactions and " << TheProgress.errs() << " errors" << endc;
	if (TheReason)
		Comment << "shutdown reason: " << TheReason << endc;

	// last minute dumps
	TheStatPhaseMgr.forceFlush();

	TheOLog.close();
	TheSmplOLog->close();

	IsClean = true;
	InShutdown = false;
}
