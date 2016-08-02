
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__APP_WORKERS_RUNNER_H
#define POLYGRAPH__APP_WORKERS_RUNNER_H

#include "base/polygraph.h"
#include "xstd/h/signal.h"
#include "xstd/String.h"
#include "xstd/Array.h"

// Starts and monitors SMP workers
class WorkersRunner {
	public:
		WorkersRunner();

		void start(int count, const int argc, char *argv[]);
		void monitor(); // wait for workers to finish, killing them when needed

		bool sawError; // whether any of the workers failed in some way

	protected:
		typedef enum {
			waitForAnyWorkerToFinish,
			waitForRemainingWorkersToFinish,
			waitForTermedWorkersToQuit,
			waitForKilledWorkersToQuit
		} WaitingState;

		static void AlarmHandler(int sig);

		void handleStoppedWorker(const pid_t pid, const int exitStatus);
		void handleAlarm();

	private:
		static bool GotAlarmSignal;

		int workerCount; // number of still running workers
		Array<pid_t> pids; // PIDs for running workers, indexed by worker ID

		WaitingState waitingState;
		typedef unsigned int WaitSeconds;
		WaitSeconds secondsToWait;

		static const WaitSeconds SecondsForRemainingWorkersToFinish = 5 * 60;
		static const WaitSeconds SecondsForTermedWorkersToQuit = 5 * 60;
		static const WaitSeconds SecondsForKilledWorkersToQuit = 5;
};

#endif
