
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include <ctype.h>

#include "xstd/Dns.h"
#include "xstd/Time.h"
#include "xstd/BigSize.h"
#include "xstd/NetAddr.h"
#include "xstd/rndDistrs.h"
#include "xstd/gadgets.h"
#include "xstd/Assert.h"
#include "pgl/PglNetAddrParts.h"
#include "pgl/pglStrIs.h"


bool pglIsRatio(const String &str, double &r) {
	if (str) {
		double h;
		const char *ptr = 0;
		if (isNum(str.cstr(), h, &ptr)) {
			if (!*ptr) { // nothing at the end
				r = h;
				return true;
			} else
			if (!strcmp(ptr, "%") || !strcmp(ptr, "p")) { // percent
				r = h / 100;
				return true;
			}
		}
	}
	return false;
}

bool pglIsRelTime(const String &str, Time &t) {
	// note: things are set up so that "sec" stands for "1sec" 
	//       (convenient for rates)
	const char *ptr = str.cstr();
	int i = 1;
	double d;

	// try to preserve integer precision if possible
	const bool isi = !isDbl(str.cstr(), d, &ptr);

	if (isi)
		isInt(str.cstr(), i, &ptr);

	if (!*ptr && isi && i == 0)
		t = Time(0,0);
	else
	if (!strcmp(ptr, "msec") || !strcmp(ptr, "ms"))
		t = isi ? Time::Msec(i) : Time::Secd(d/1e3);
	else
	if (!strcmp(ptr, "sec") || !strcmp(ptr, "s"))
		t = isi ? Time::Sec(i) : Time::Secd(d);
	else
	if (!strcmp(ptr, "min"))
		t = isi ? Time::Sec(i*60) : Time::Mind(d);
	else
	if (!strcmp(ptr, "hour") || !strcmp(ptr, "hr"))
		t = isi ? Time::Sec(i*60*60) : Time::Hourd(d);
	else
	if (!strcmp(ptr, "day"))
		t = isi ? Time::Sec(i*24*60*60) : Time::Dayd(d);
	else
	if (!strcmp(ptr, "year"))
		t = isi ? Time::Sec(i*365*24*60*60) : Time::Yeard(d);
	else
		return false;

	return true;
}

bool pglIsAbsTime(const String &str, Time &t) {
	struct tm h;
	memset(&h, 0, sizeof(h));

	const char *eos = str.cstr() + str.len();

	// check for day: YYYY/MM/DD
	const char *p = str.chr('/');
	if (!p || (p-=4) < str.cstr() || p+9 >= eos)
		return false;
	
	h.tm_year = xatoi(p, -1) - 1900;
	h.tm_mon = xatoi(p+=5, -1) - 1;
	h.tm_mday = xatoi(p+=3, -1);

	if (h.tm_year <= 0 || h.tm_mon < 0 || h.tm_mday <= 0)
		return false;

	// check for time: HH:MM:SS
	p = str.chr(':');
	if (p) {
		if ((p-=2) < str.cstr() || p+4 >= eos)
			return false;

		h.tm_hour = xatoi(p, -1);
		h.tm_min = xatoi(p+=3, -1);
		if ((p+=2) < eos && *p == ':')
			h.tm_sec = xatoi(++p, -1);

		if (h.tm_hour < 0 || h.tm_min < 0 || h.tm_sec < 0)
			return false;
	}

	Time tt(h);
	if (tt >= 0) {
		t = tt;
		return true;
	}

	return false;
}

bool pglIsSize(const String &str, BigSize &s) {
	// note: things are set up so that "KB" stands for "1KB" 
	//       (convenient for rates)
	const char *ptr = str.cstr();
	int i = 1;
	double d = 1;
	bool isi = false;

	if (isDbl(str.cstr(), d, &ptr))
		i = (int)rint(d);
	else
	if ((isi = isInt(str.cstr(), i, &ptr)))
		d = i;

	if (isi && !*ptr && i == 0)
		s = 0;
	else
	if (!strcmp(ptr, "bit")) {
		if (i % 8 == 0) {
			s = i/8;
		} else {
			cerr << here <<
				"bit sizes not divisible by 8 are not supported; " <<
				" found: " << str << endl << xexit;
			return false; // not reached
		}
	} else
	if (!strcmp(ptr, "Byte"))
		s = i;
	else
	if (!strcmp(ptr, "Kb") || !strcmp(ptr, "Kbit"))
		s = ((int)(1024*d/8));
	else
	if (!strcmp(ptr, "KB") || !strcmp(ptr, "KByte"))
		s = ((int)(1024*d));
	else
	if (!strcmp(ptr, "Mb") || !strcmp(ptr, "Mbit"))
		s = BigSize::MBd(d/8);
	else
	if (!strcmp(ptr, "MB") || !strcmp(ptr, "MByte"))
		s = isi ? BigSize::MB((int)d) : BigSize::MBd(d);
	else
	if (!strcmp(ptr, "Gb") || !strcmp(ptr, "Gbit"))
		s = BigSize::MBd(1024*d/8);
	else
	if (!strcmp(ptr, "GB") || !strcmp(ptr, "GByte"))
		s = BigSize::MBd(1024*d);
	else
		return false;

	return true;
}

bool pglIsNetAddr(const String &str, NetAddr &a) {
	PglNetAddrParts parts(str);
	if (parts.error() || !parts.single())
		return false;

	a = NetAddr(parts.host(), parts.port());
	return true;
}
