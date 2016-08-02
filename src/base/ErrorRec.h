
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_ERRORREC_H
#define POLYGRAPH__BASE_ERRORREC_H

#include "xstd/Error.h"
#include "xstd/String.h"
#include "base/LogObj.h"

// holds info about a particular error
class ErrorRec: public Error {
	public:
		ErrorRec();
		ErrorRec(const ErrorRec &rec);
		ErrorRec(const Error &e);
		~ErrorRec();

		void reset();

		bool needReport() const;
		Counter count() const { return theCount; }
		
		void noteError(const Counter count = 1) { theCount += count; }
		void noteReport(const Time &tout);

		void store(OLog &ol) const;
		void load(ILog &il);

		void add(const ErrorRec &e);

		ErrorRec *&next() { return theNext; }

		ostream &print(ostream &os) const { return Error::print(os, theText.cstr()); }

	protected:
		Counter theCount; // number of occurencies

		Counter theRepCount; // report when count is reached
		Time theRepTime; // report when time is reached

		String theText; // error text

		ErrorRec *theNext; // next error in a bucket
};

inline
ostream &operator <<(ostream &os, const ErrorRec &err) {
	return err.print(os);
}

#endif
