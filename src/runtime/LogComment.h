
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_LOGCOMMENT_H
#define POLYGRAPH__RUNTIME_LOGCOMMENT_H

#include "xstd/h/sstream.h"

#include "base/OLog.h"

/* useful to write comments (free style messages) to a log file
 *  usage:
 *          Comment(level) << ... << endc;
 */

class LogComment: public ofixedstream {
	public:
		LogComment();

		void reset() { clear(); seekp(0); TheCurLevel = 0; }

		LogComment &operator ()(int aLevel) { TheCurLevel = aLevel; return *this; }

		const char *str();

	public:
		static int TheEchoLevel;
		static int TheCurLevel;

	protected:
		static char TheBuf[8*1024];
};

extern LogComment Comment;
extern ostream &endc(ostream &os);

#endif
