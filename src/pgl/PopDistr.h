
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_POPDISTR_H
#define POLYGRAPH__PGL_POPDISTR_H

class RndGen;

// popularity distribution interface

class PopDistr {
	public:
		virtual ~PopDistr() {}

		// selects an object name from [1, lastName] range
		virtual int choose(RndGen &rng, int lastName) = 0;

		virtual ostream &print(ostream &os) const = 0;
};

#endif
