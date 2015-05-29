
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>

#include "base/polyOpts.h"
#include "runtime/LogComment.h"
#include "runtime/SharedOpts.h"
#include "app/DebugSwitch.h"


void DebugSignal(int s) {
	clog << "got debug signal (" << s << ')';

	static bool enable = true;
	static DumpFlagsOpt opts(TheOpts.theDumpFlags);
	static int echoLevel = -1;

	if (enable) {
		clog << " detailed debugging enabled";

		opts = TheOpts.theDumpFlags;
		echoLevel = Comment.TheEchoLevel;

		TheOpts.theDumpFlags.setAll();
		Comment.TheEchoLevel = INT_MAX;
	} else {
		clog << " reverting to configured debugging level";

		TheOpts.theDumpFlags = opts;
		Comment.TheEchoLevel = echoLevel;
	}

	clog << endl;
	enable = !enable;
}
