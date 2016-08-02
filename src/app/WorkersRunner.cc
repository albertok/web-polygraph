
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/signal.h"
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"
#include "xstd/h/os_std.h"

#include <sys/wait.h>

#include "xstd/AlarmClock.h"
#include "base/AnyToString.h"
#include "runtime/LogComment.h"
#include "app/shutdown.h"
#include "app/WorkersRunner.h"

#ifndef WCOREDUMP
#	define WCOREDUMP(x) false
#endif


bool WorkersRunner::GotAlarmSignal = false;

void WorkersRunner::AlarmHandler(int sig) {
	GotAlarmSignal = true;
}

WorkersRunner::WorkersRunner():
	sawError(false),
	workerCount(0),
	waitingState(waitForAnyWorkerToFinish),
	secondsToWait(0) {
}

void WorkersRunner::start(const int startCount, const int argc, char *argv[]) {
	Comment(6) << "fyi: starting " << startCount << " workers" << endc;

	const Area macros(Area::Create("%worker"));
	for (int i = 1; i < argc - 1; ++i) {
		String opt(argv[i]);
		if (opt == "--console" || opt == "--log") {
			String val(argv[i + 1]);
			if (val.find(macros) == String::npos)
				cerr << opt << " option values do not use macros %worker" << endl << xexit;
		}
	}

	char *warg[argc + 3]; // worker command line arguments
	warg[0] = 0; // worker process name; to be set before execv()
	warg[1] = const_cast<char*>("--worker");
	warg[2] = 0; // worker ID; to be set before execv()
	for (int i = 1; i < argc; ++i)
		warg[i + 2] = argv[i];
	warg[argc + 2] = 0;

	const String progName = argv[0];
	for (int i = 1; i <= startCount; ++i) {
		const pid_t pid = fork();
		if (pid == 0) {
			const String workerId = AnyToString(i);
			const String processName = "(" + progName + "-" + workerId + ")";
			warg[0] = const_cast<char*>(processName.cstr());
			warg[2] = const_cast<char*>(workerId.cstr());

			execv(argv[0], warg);
			cerr << argv[0] << " " << warg[1] << " " << warg[2] <<
				" cannot start: " << Error::Last() << endl << xexit;
		} else {
			pids.append(pid);
			Clock::Update(false);
			Comment(1) << "fyi: worker " << i << " (PID " << pid << ") started" << endc;
		}
	}

	workerCount = pids.count();
}

void WorkersRunner::monitor() {
	alarm(0); // clear any previosuly set alarm (there should not be any)
	struct sigaction act;
	sigaction(SIGALRM, NULL, &act); // get old/default action settings
	act.sa_handler = &WorkersRunner::AlarmHandler;
	act.sa_flags = 0;
	sigaction(SIGALRM, &act, NULL);

	while (workerCount > 0) {
		if (waitingState > waitForAnyWorkerToFinish) {
			ShouldUs(secondsToWait > 0); // alarm(0) below should not return 0
			alarm(max(1U, secondsToWait)); // do not wait forever
		}

		int exitStatus = 0;
		const pid_t pid = wait(&exitStatus);
		const int savedErrno = errno;

		if (waitingState > waitForAnyWorkerToFinish)
			secondsToWait = alarm(0); // wait time remaining
		Clock::Update(false);

		if (pid > 0) { // a worker has exited
			handleStoppedWorker(pid, exitStatus);
		} else
		if (ShouldSys(pid < 0 && savedErrno == EINTR)) {
			ShouldUs(GotAlarmSignal);
			// do nothing; we woke up on an alarm that will be processed below
		} else {
			throw "workers monitoring failure";
		}

		if (GotAlarmSignal) {
			GotAlarmSignal = false;
			handleAlarm();
		}
	}

	ShouldUs(!workerCount);
	ShutdownReason((waitingState <= waitForRemainingWorkersToFinish) ?
		"all workers stopped" : "some workers stopped; others were killed");
}

void WorkersRunner::handleStoppedWorker(const pid_t pid, const int exitStatus) {
	int idx = -1;
	if (!pids.find(pid, idx)) {
		Comment(1) << "warning: unknown worker quit, PID: " << pid << endc;
		return;
	}

	const String workerName = "worker " + AnyToString(idx+1) +
		" (PID " + AnyToString(pid) + ")";
	if (WIFEXITED(exitStatus)) {
		if (const int ec = WEXITSTATUS(exitStatus)) {
			Comment(1) << "error: " << workerName << 
				" terminated with error code " << ec << endc;
			sawError = true;
		} else
			Comment(2) << "fyi: " << workerName << " exited normally" << endc;
	} else
	if (WIFSIGNALED(exitStatus)) {
		ostream &os = Comment(1) << "error: " << workerName <<
			" terminated by signal " << WTERMSIG(exitStatus);
		if (WCOREDUMP(exitStatus))
			os << ", core dumped";
		os << endc;
		sawError = true;
	} else {
		// unknown termination reason
		Comment(1) << "error: " << workerName << " terminated" << endc;
		sawError = true;
	}

	pids[idx] = 0;
	--workerCount;

	if (workerCount > 0 && waitingState == waitForAnyWorkerToFinish) {
		waitingState = waitForRemainingWorkersToFinish;
		secondsToWait = SecondsForRemainingWorkersToFinish;
		// TODO: time in messages may get out of sync; use/log Time instead?
		Comment(5) << "starting a 5-minute wait for remaining " <<
			workerCount	<< " worker(s) to terminate" << endc;
	}
}

void WorkersRunner::handleAlarm() {
	switch (waitingState) {
		case waitForAnyWorkerToFinish:
			ShouldUs(false);
			break;

		case waitForRemainingWorkersToFinish:
			Comment(1) << "stopping all " << workerCount
				<< " workers remaining after a 5-minute wait" << endc;
			for (int i = 0; i < pids.count(); ++i)
				if (pids[i])
					kill(pids[i], SIGTERM);
			waitingState = waitForTermedWorkersToQuit;
			secondsToWait = SecondsForTermedWorkersToQuit;
			break;

		case waitForTermedWorkersToQuit:
			Comment(0) << "killing all " << workerCount
				<< " workers remaining after a 10-minute wait" << endc;
			for (int i = 0; i < pids.count(); ++i)
				if (pids[i])
					kill(pids[i], SIGKILL);
			waitingState = waitForKilledWorkersToQuit;
			secondsToWait = SecondsForKilledWorkersToQuit;
			break;

		case waitForKilledWorkersToQuit:
			throw "some workers stopped; others refuse to die";
	}
}
