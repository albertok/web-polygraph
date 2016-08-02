
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_ERRORSTAT_H
#define POLYGRAPH__BASE_ERRORSTAT_H

#include "base/LogObj.h"

class Error;
class ErrorRec;
class ErrorHash;

// detailed error histogram (via hash of occured errors)
class ErrorStat: public LogObj {
	public:
		typedef Array<const ErrorRec*> Index;

	public:
		ErrorStat();
		virtual ~ErrorStat();

		void reset();
		Counter count() const { return theCount; }
		Counter count(const Error &e) const;

		int index(Index &idx) const;

		// returns whether a report is expected
		bool record(const Error &e, ErrorRec *&rec, const Counter count = 1);

		void add(const ErrorStat &s);
		ErrorStat &operator =(const ErrorStat &es);

		virtual OLog &store(OLog &ol) const;
		virtual ILog &load(ILog &il);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		void newHash();

	protected:
		ErrorHash *theHash;
		Counter theCount; // total number of errors
};

#endif
