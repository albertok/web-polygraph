
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "xstd/xstd.h"

#include <limits.h>
#include <stdlib.h>
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Assert.h"
#include "xstd/Time.h"
#include "xstd/xport.h"
#include "xstd/gadgets.h"


#if !defined(HAVE_GETTIMEOFDAY) && defined(HAVE__FTIME)
#include <sys/timeb.h>
static
void gettimeofday(Time *tm, void *) {
	struct _timeb tb;
	_ftime(&tb);
	tm->tv_sec  = tb.time;
	tm->tv_usec = tb.millitm;
}
#endif


Time Time::Now() {
	Time tm;
	gettimeofday(&tm, 0);

	// Some Linux kernels have gettimeofday bugs that cause time jumps
	// of approximately 72 minutes, especially on SMP systems. We saw it
	// on an 8-CPU box running 2.6.18-8.el5. For 2002 discussion, start at
	// http://www.kernel-traffic.org/kernel-traffic/kt20020708_174.html#1
	// Here, we try to ignore individual jumps exceeding 60 minutes.
	static Time ptm = tm; // previous time
	if (tm.tv_sec - ptm.tv_sec > 3600L) {
		clog << "Warning: ignoring " << (tm - ptm).secd() << 
			"sec time jump" << endl <<
			"\tfrom: " << ptm.tv_sec <<'.'<< ptm.tv_usec << endl <<
			"\tto:   " << tm.tv_sec << '.' << tm.tv_usec << endl;
		const Time result = ptm;
		ptm = tm; // if the strange value persists, honor it
		return result;
	}
	ptm = tm;

	return tm;
}

Time Time::Max() {
	return Time(INT_MAX, INT_MAX);
}

Time Time::Msec(long msec) {
	return Time(msec / 1000, 1000*(msec % 1000));
}

Time Time::Secd(double dsec) {
	if (!Should(dsec <= LONG_MAX))
		dsec = LONG_MAX;
	else
	if (!Should(dsec >= LONG_MIN))
		dsec = LONG_MIN;
	const long sec = (long)dsec;
	return Time(sec, (long)((dsec-sec)*1e6));
}

Time::Time(const struct tm &t) {
	struct tm t2 = t;    // must copy -- timegm modifies its param
	if (t2.tm_year < 70) // a Y2K plug-in!
		t2.tm_year += 100;
	tv_sec = (0 <= t2.tm_mon && t2.tm_mon < 12) ? xtimegm(&t2) : -1;
	tv_usec = tv_sec >= 0 ? 0 : -1;
}

Time &Time::operator +=(const Time &tm) {

	tv_sec += tm.tv_sec;
	tv_usec += tm.tv_usec;

	if (tv_usec >= 1000000L) {
		tv_usec -= 1000000L;
		tv_sec++;
	}

	return *this;
}

// note: negative times are confusing for humans; e.g., "-1:1" means "-0.1"
Time &Time::operator -=(const Time &tm) {

	tv_sec -= tm.tv_sec;
	tv_usec -= tm.tv_usec;

	if (tv_usec < 0) {
		tv_usec += 1000000L;
		tv_sec--;
	}

	return *this;
}

Time &Time::operator *=(int factor) {
	tv_sec *= factor;
	tv_usec *= factor;

	if (tv_usec >= 1000000L) {
		tv_sec += tv_usec/1000000L;
		tv_usec %= 1000000L;
	} else
	if (tv_usec < 0) {
		tv_usec = -tv_usec;
		tv_sec -= tv_usec/1000000L;
		tv_sec--;
		tv_usec = 1000000L - (tv_usec % 1000000L);
	}

	return *this;
}

Time &Time::operator /=(double factor) {
	if (factor) {
		const double dsec = secd()/factor;
		tv_sec = (long) dsec;
		tv_usec = (long)((dsec-tv_sec)*1e6);
	} else {
		tv_sec = tv_usec = -1;
	}

	return *this;
}

struct tm *Time::gmtime() const {
	time_t clock = (time_t) tv_sec;
	return ::gmtime(&clock);
}

istream &Time::read(istream &is) {
	char c;
	return is >> tv_sec >> c >> tv_usec;
}

ostream &Time::print(ostream &os) const {
	if (tv_sec < 60*60*24*365 && tv_usec >= 0)
		return printInterval(os);
	if (tv_sec == -1 && tv_usec == -1)
		return os << "<none>"; // works for everybody?

	const char osfill = os.fill();
	return os << tv_sec << '.' << setw(6) << setfill('0') << tv_usec << setfill(osfill);
}

ostream &Time::printInterval(ostream &os) const {

	const int osprec = os.precision(2);
	const char osfill = os.fill();

	if (tv_sec < 0)
		os << tv_sec << '.' << setfill('0') << setw(6) << tv_usec << "sec";
	else
	if (tv_sec == 0)
		if (tv_usec < 1000)
			os << tv_usec << "usec";
		else
			os << (tv_usec/1000) << "msec";
	else
	if (tv_sec < 60)
		os << secd() << "sec";
	else
	if (tv_sec < 60*60)
		os << secd()/60 << "min";
	else
	if (tv_sec < 24*60*60)
		os << secd()/(60*60) << "hour";
	else
	if (tv_sec < 365*24*60*60)
		os << secd()/(24*60*60) << "day";
	else
		os << secd()/(365*24*60*60) << "year";

	os.fill(osfill);
	os.precision(osprec);

	return os;
}

// useful for debugging
ostream &operator <<(ostream &os, const struct tm &t) {
	return os << here << endl
		<< '\t' << "tm_mday: " << "\t " << setw(4) << t.tm_mday << endl
		<< '\t' << "tm_mon:  " << "\t " << setw(4) << t.tm_mon << endl
		<< '\t' << "tm_year: " << "\t " << setw(4) << t.tm_year << endl
		<< '\t' << "tm_hour: " << "\t " << setw(4) << t.tm_hour << endl
		<< '\t' << "tm_min:  " << "\t " << setw(4) << t.tm_min << endl
		<< '\t' << "tm_sec:  " << "\t " << setw(4) << t.tm_sec << endl
		<< '\t' << "tv_sec:  " << "\t " << setw(4) << Time(t) << endl
		<< endl;
}
