
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_SOCKETSYM_H
#define POLYGRAPH__PGL_SOCKETSYM_H

#include "pgl/PglRecSym.h"

class Time;

// console configuration
class SocketSym: public RecSym {
	public:
		static String TheType;

	public:
		SocketSym();
		SocketSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		bool lingerTout(Time &tout) const;
		bool nagle(bool &doNagle) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
