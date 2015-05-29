
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__APP_SHUTDOWN_H
#define POLYGRAPH__APP_SHUTDOWN_H

// routines that determine shutdown conditions, report them,
// and assist in nice program termination

class String;

// stop now? (checks various conditions)
extern bool ShutdownNow();

// returns true iff ShutdownNow returned true before
extern bool ShutdownRequested();

// note a shutdown signal
extern void ShutdownSignal(int s);

// force a shutdown for some reason
extern void ShutdownReason(const String &reason);

// to be registered with at_exit
extern void ShutdownAtExit();

// to be registered with set_new_handler
extern void ShutdownAtNew();

#endif
