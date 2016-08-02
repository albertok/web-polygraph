
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_SSLPHASESTAT_H
#define POLYGRAPH__BASE_SSLPHASESTAT_H

#include "base/LogObj.h"

// collects new/reused SSL session statistics
class SslPhaseStat: public LogObj {
	public:
		typedef enum { skNew, skReused } SessionKind;

		class Stat: public LogObj {
			public:
				Stat();

				Counter newSessions() const { return theNew; }
				Counter reusedSessions() const { return theReused; }
				Counter allSessions() const { return theNew + theReused; }

				void record(const SessionKind kind);

				Stat &operator +=(const Stat &s);

				virtual OLog &store(OLog &log) const;
				virtual ILog &load(ILog &log);

				ostream &print(ostream &os, const String &pfx) const;

			protected:
				Counter theNew; // new SSL sessions count
				Counter theReused; // reused SSL sessions count
		};

	public:
		const Stat &offered() const { return theOffered; }
		Stat &offered() { return theOffered; }
		const Stat &measured() const { return theMeasured; }
		Stat &measured() { return theMeasured; }

		SslPhaseStat &operator +=(const SslPhaseStat &s);

		virtual OLog &store(OLog &log) const;
		virtual ILog &load(ILog &log);

		ostream &print(ostream &os, const String &pfx) const;

	protected:
		Stat theOffered; // offered SSL sessions stats
		Stat theMeasured; // measured SSL session stats
};

#endif
