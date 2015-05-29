
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_OBJTIMES_H
#define POLYGRAPH__BASE_OBJTIMES_H

class ObjLifeCycle;
class OLog;

// holds various object (entity) timestamps
class ObjTimes {
	friend class ObjLifeCycle;

	public:
		ObjTimes() { reset(); }

		void reset();

		Time lmt() const { return theLmt; }
		Time exp() const { return theExp; }
		bool showLmt() const { return doShowLmt; }
		bool knownExp() const { return theExp >= 0; }

		void store(OLog &log) const;

	protected:
		Time theLmt;
		Time theExp;
		bool doShowLmt;
};

#endif
