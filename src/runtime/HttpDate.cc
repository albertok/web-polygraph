
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include "xstd/h/string.h"
#include "xstd/h/sstream.h"
#include "xstd/h/iomanip.h"

#include "xstd/Clock.h"
#include "runtime/HttpDate.h"


// Map: three letter (or more) month -> month id
// note: optimization assumes HTTP Date formats!
static class MonthMap {
	public:
		MonthMap();

		int id(const char *mon) const { return check(mon, mon2id(mon)); }
		const char *month(int id) const { return 0 <= id && id <= 11 ? theMonths[id] : 0; }

	protected:
		int mon2idx(const unsigned char *mon) const { return ((int)mon[1]) + (int)mon[2]; }
		int mon2id(const char *mon) const { return theMap[mon2idx((const unsigned char*)mon)]; }
		int check(const char *mon, int id) const { return id >= 0 && !strncasecmp(mon, month(id), 3) ? id : -1; }

	protected:
		const char *theMonths[12];
		int theMap[255+255];
} TheMonthMap;

static const char *TheWeekDays[7] = {
	"Sun", "Mon", "Tue", "Wed",  "Thu", "Fri", "Sat" };

MonthMap::MonthMap() {
	// init map with invalid ids
	const int mapCap = sizeof(theMap)/sizeof(*theMap);
	{for (int i = 0; i < mapCap; ++i)
		theMap[i] = -1;
	}

	// set month names
	const char *months[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	for (int i = 0; i < 12; ++i) {
		const int idx = mon2idx((const unsigned char*)months[i]);
		//cerr << here << i << ". " << idx << endl;
		Assert(0 <= idx && idx < mapCap); // within the range 
		Assert(theMap[idx] == -1); // no duplicates
		theMap[idx] = i;
		theMonths[i] = months[i];
	}

	// P.S. id() = ([1] >> 2) ^ ([2] << 2);  also works
}

/* various inlined number parsers */

inline
int parseNum1(const char *&date) {
	return (int)(*date++) - (int)('0');
}

inline
int parseNum2(const char *&date) {
	return 10*parseNum1(date) + parseNum1(date);
}

inline
int parseNum4(const char *&date) {
	return 100*parseNum2(date) + parseNum2(date);
}

inline // " 6" or "16"
int parseNumFlex(const char *&date) {
	return isdigit(*date) ? parseNum2(date) : parseNum1(++date);
}

inline
int parseMonth(const char *&date) {
	const int mon = TheMonthMap.id(date); 
	date += 3;
	return mon;
}

// assumes preprocessing done in ParseHttpDate()
static
Time parseRfc1123(const char *date) {
	static struct tm t;

	t.tm_mday = parseNum2(date); ++date;
	t.tm_mon = parseMonth(date); ++date;
	t.tm_year = parseNum4(date) - 1900; ++date;
	t.tm_hour = parseNum2(date); ++date;
	t.tm_min = parseNum2(date); ++date;
	t.tm_sec = parseNum2(date);
	t.tm_isdst = -1;

	return Time(t);
}


// assumes preprocessing done in ParseHttpDate()
static
Time parseRfc850(const char *date) {
	// skip "Sunday, " till the first digit
	while (*date && !isdigit(*date)) ++date;

	static struct tm t;
	
	t.tm_mday = parseNum2(date); ++date;
	t.tm_mon = parseMonth(date); ++date;
	t.tm_year = parseNum2(date); ++date;
	t.tm_hour = parseNum2(date); ++date;
	t.tm_min = parseNum2(date); ++date;
	t.tm_sec = parseNum2(date); ++date;

	return Time(t);
}

// assumes preprocessing done in ParseHttpDate()
static
Time parseAnsiDate(const char *date) {
	static struct tm t;

	t.tm_mon = parseMonth(date); ++date;
	t.tm_mday = parseNumFlex(date); ++date;
	t.tm_hour = parseNum2(date); ++date;
	t.tm_min = parseNum2(date); ++date;
	t.tm_sec = parseNum2(date); ++date;
	t.tm_year = parseNum4(date) - 1900;
	
	return Time(t);
}

Time HttpDateParse(const char *date, int len) {
	/* the following formats are parse-musts in HTTP; 
	 * the first format is a generate-must in HTTP:
	 *
	 *  Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
	 *  Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
	 *  Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
	 */

	if (len < 23)
		return Time();

	date += 3;
	if (*date == ',')
		return parseRfc1123(date+2);
	else
	if (!isspace(*date))
		return parseRfc850(date+2);
	else
		return parseAnsiDate(date+1);
}

ostream &HttpDatePrint(ostream &os, Time date) {
	if (struct tm *t = date.gmtime()) {
		const char fill = os.fill('0');
		return os
			<< TheWeekDays[t->tm_wday] << ", "
			<< setw(2) << t->tm_mday << ' '
			<< TheMonthMap.month(t->tm_mon) << ' '
			<< (t->tm_year + 1900) << ' '
			<< setw(2) << t->tm_hour << ':'
			<< setw(2) << t->tm_min << ':'
			<< setw(2) << t->tm_sec << " GMT"
			<< setfill(fill);
	} else
		return os << "<none>";
}

// prints and caches current date
ostream &HttpDatePrint(ostream &os) {
	static char cachedImage[30];
	static Time cachedDate;

	if (cachedDate.tv_sec != TheClock.time().tv_sec) {
		// can we just reset it instead of re-creating?
		ofixedstream str(cachedImage, sizeof(cachedImage));
		HttpDatePrint(str, cachedDate = TheClock);
	}
	// assume that we cached a valid, full-length date
	os.write(cachedImage, sizeof(cachedImage)-1);
	return os;
}



#if TEST_HTTP_DATE
//g++ -o x HttpDate.cc -Iinclude -Ixstd/include xstd/libxstd.a
main(int argc, char *argv[]) {
	HttpDatePrint(cout << here) << endl;

	for (int i = 1; i < argc; ++i) {
		const Time t = ParseHttpDate(argv[i], strlen(argv[i]));
		cout << argv[i] << endl;
		HttpDatePrint(cout, t) << endl;
	}
}
#endif
