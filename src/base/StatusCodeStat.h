
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__BASE_STATUSCODESTAT_H
#define POLYGRAPH__BASE_STATUSCODESTAT_H

#include "xstd/Array.h"
#include "base/LogObj.h"

class TmSzStat;

// size and response size for response status codes
class StatusCodeStat: public LogObj {
	public:
		enum {
			scsMinValue = -1,  // minimum status code value, see RepHdr::scUnknown
			scsMaxValue = 999, // maximum status code value
			scsOther           // index for the scsMinValue
		};

		StatusCodeStat();
		virtual ~StatusCodeStat();

		TmSzStat allStats() const;
		const TmSzStat *stats(int status) const;

		void record(int status, const Time &tm, Size sz);
		void add(const StatusCodeStat &s);

		virtual OLog &store(OLog &ol) const;
		virtual ILog &load(ILog &il);

		ostream &print(ostream &os, const String &pfx) const;

	protected:

		Array<TmSzStat*> theStats;
};

#endif
