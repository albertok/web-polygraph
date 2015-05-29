/*
 * %%nomodsWarning
 */

/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLLEXER_H
#define POLYGRAPH__PGL_PGLLEXER_H

#include "xparser/GLexer.h"

class PglLexer: public GLexer {
	public:
		PglLexer(istream &aSource, const String &aFName);

	protected:
		virtual int transition(int, int);

};


#endif
