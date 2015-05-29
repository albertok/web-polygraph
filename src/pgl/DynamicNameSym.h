
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_DYNAMICNAMESYM_H
#define POLYGRAPH__PGL_DYNAMICNAMESYM_H


#include "pgl/PglNetAddrSym.h"

// *.domain:port network name with new prefix probability

class DynamicNameSym: public NetAddrSym {
	public:
		static String TheType;

	public:
		DynamicNameSym();
		DynamicNameSym(const String &aType, PglRec *aRec);

		virtual bool isA(const String &type) const;

		void val(const NetAddr &addr);

		void setNewProb(const double prob);
		bool newProb(double &prob) const;

	protected:
		virtual SynSym *dupe(const String &dType) const;
};

#endif
