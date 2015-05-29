
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Clock.h"
#include "runtime/LogComment.h"

#include "runtime/PolyOLog.h"
#include "base/polyLogCats.h"
#include "base/polyLogTags.h"
#include "xstd/gadgets.h"
#include "runtime/globals.h"

int LogComment::TheEchoLevel = 0;
int LogComment::TheCurLevel = 0;
char LogComment::TheBuf[8*1024];

LogComment Comment; // note: assumes Comment is not used in static-ctors

LogComment::LogComment(): ofixedstream(TheBuf, sizeof(TheBuf)) {
	configureStream(*this, 2);
}

const char *LogComment::str() {
	flush();
	TheBuf[sizeof(TheBuf)-1] = '\0'; // make sure .str() is terminated
	return TheBuf;
}


ostream &endc(ostream &os) {
	os << ends;
	const char *c = Comment.str();
	TheOLog << bege(lgComment, lgcAll) << c << ende;
	if (Comment.TheCurLevel <= Comment.TheEchoLevel) {
		cout 
			<< setfill('0') << setw(6) << (TheClock.runTime().secd()/60)
			<< setfill(' ')	<< "| "
			<< c << endl;
	}
	Comment.reset();
	return os;
}
