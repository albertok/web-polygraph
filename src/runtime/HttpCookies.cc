
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "runtime/HttpCookies.h"
#include "runtime/HttpDate.h"
#include "runtime/LogComment.h"
#include "xstd/gadgets.h"
#include "xstd/Clock.h"
#include "xstd/PrefixIdentifier.h"


/* HttpCookie */

ObjFarm<HttpCookie> HttpCookie::TheCookieFarm;

HttpCookie::HttpCookie() {
	reset();
}

void HttpCookie::reset() {
	theName = theData = String();
	theExpires = Time();
	isDiscardable = false;
}

void HttpCookie::set(const String &aData, const int nameEnd) {
	Assert(!theData && aData);
	Assert(!theName);
	Assert(nameEnd > 0 && nameEnd != String::npos);
	theData = aData;
	theName = theData(0, nameEnd);
}

void HttpCookie::maxAge(const int delta) {
	theExpires = TheClock.time() + Time::Sec(delta);
}

bool HttpCookie::expired() const {
	return theExpires != Time() && theExpires <= TheClock.time();
}

bool HttpCookie::discardable() const {
	return isDiscardable || theExpires == Time() || expired();
}

void HttpCookie::Configure() {
	TheCookieFarm.limit(1024); // magic, no good way to estimate
}

void HttpCookie::Put(HttpCookie *const cookie) {
	TheCookieFarm.put(cookie);
}

HttpCookie *HttpCookie::Parse(const char *buf, const char *eoh) {
	HttpCookie *cookie(0);
	// skip spaces and newlines at the end
	while (buf < eoh && isspace(eoh[-1])) --eoh;
	// loop through all name=value pairs
	// the first pair is stored as opaque cookie data
	while (buf < eoh) {
		const char *end(eoh);
		const char *const semicolon = StrBoundChr(buf, ';', end);
		if (semicolon) {
			const char *quote = StrBoundChr(buf, '"', end);
			if (!quote || semicolon < quote)
				end = semicolon;
			else {
				quote = StrBoundChr(quote + 1, '"', end);
				if (quote)
					end = quote + 1;
			}
		}

		if (!cookie) {
			const String data(buf, end - buf);
			const int nameEnd(data.find('='));
			if (Should(nameEnd != String::npos)) {
				cookie = TheCookieFarm.get();
				cookie->set(data, nameEnd);
			} else
				break;
		} else
		if (!ParseParameter(buf, end, *cookie)) {
			Put(cookie);
			cookie = 0;
			break;
		}

		// skip until the next name=value pair begins
		buf = end;
		while (buf < eoh && (isspace(*buf) || *buf == ';')) ++buf;
	}

	return cookie;
}

bool HttpCookie::ParseParameter(const char *const buf, const char *const end, HttpCookie &cookie) {
	enum {
		pUnknown,
		pDiscard,
		pExpires,
		pMaxAge
	};
	static const String Discard("Discard");
	static const String Expires("expires=");
	static const String MaxAge("Max-Age=");
	static PrefixIdentifier sidf;
	if (!sidf.count()) {
		sidf.add(Discard, pDiscard);
		sidf.add(Expires, pExpires);
		sidf.add(MaxAge, pMaxAge);
		sidf.optimize();
	}
	bool result(true);
	switch (sidf.lookup(buf, end - buf)) {
		case pDiscard:
			if (buf + sizeof(Discard) == end)
				cookie.discard();
			else
				result = true;
			break;
		case pExpires: {
			const Time date(HttpDateParse(buf + Expires.len(), end - buf - Expires.len()));
			if (Should(date >= 0))
				cookie.expires(date);
			else
				result = true;
			break;
		}
		case pMaxAge: {
			int delta;
			const char *p;
			if (Should(isInt(buf + MaxAge.len(), delta, &p, 10) &&
				p == end))
				cookie.maxAge(delta);
			else
				result = true;
			break;
		}
		case pUnknown:
			// ignore other parameters
			break;
	}
	return result;
}


/* HttpCookies */

Counter HttpCookies::TheTotalCount(0);
Counter HttpCookies::ThePurgedFreshCount(0);
Counter HttpCookies::ThePurgedStaleCount(0);
Counter HttpCookies::TheUpdatedCount(0);


HttpCookies::HttpCookies(): theLimit(0) {
}

HttpCookies::~HttpCookies() {
	for (Cache::const_iterator i = theCookies.begin();
		i != theCookies.end();
		++i)
		HttpCookie::Put(*i);
}

void HttpCookies::keepLimit(const unsigned limit) {
	Assert(!theLimit && limit);
	theLimit = limit;
	while (theCookies.size() > theLimit)
		purgeAt(theCookies.end());
}

void HttpCookies::add(HttpCookie *const cookie) {
	Assert(cookie);

	const String name(cookie->name());
	if (cookie->expired()) {
		NameMap::iterator i(theNameMap.find(name));
		if (i != theNameMap.end()) {
			HttpCookie::Put(*i->second);
			// so that purge stats use the new value
			*i->second = cookie;
			purgeAt(i->second, false);
			theNameMap.erase(i);
		}
	} else {
		theCookies.push_front(cookie);
		std::pair<NameMap::iterator, bool> result =
			theNameMap.insert(std::make_pair(name, theCookies.begin()));
		if (result.second) {
			// new cookie was added
			if (theLimit && theCookies.size() > theLimit)
				purgeAt(theCookies.end());
			reportCached(*cookie);
		} else {
			// existing cookie updated
			// result.first->second is the iterator pointing
			// to the cookie being updated
			// purge the old cookie from the cache
			purgeAt(result.first->second, false, false);
			// put reference to the new cookie into the name map
			result.first->second = theCookies.begin();
			reportUpdated(*cookie);
		}
	}
}

int HttpCookies::count() const {
	return theCookies.size();
}

const HttpCookie *HttpCookies::first() {
	theIter = theCookies.begin();
	return sync();
}

const HttpCookie *HttpCookies::next() {
	Should(theIter != theCookies.end());
	++theIter;
	return sync();
}

void HttpCookies::purgeDiscardable() {
	theIter = theCookies.begin();
	while (theIter != theCookies.end()) {
		if ((*theIter)->discardable())
			purgeAt(theIter++);
		else
			++theIter;
	}
}

void HttpCookies::purgeAt(const Cache::iterator &i, const bool purgeName, const bool doReport) {
	HttpCookie *const cookie(*i);
	if (doReport)
		reportPurged(*cookie);
	if (purgeName)
		theNameMap.erase(cookie->name());
	theCookies.erase(i);
	HttpCookie::Put(cookie);
}

const HttpCookie *HttpCookies::sync() {
	const HttpCookie *cookie(0);
	while (!cookie && theIter != theCookies.end()) {
		cookie = *theIter;
		if (cookie->expired()) {
			purgeAt(theIter++);
			cookie = 0;
		}
	}
	return cookie;
}

void HttpCookies::reportCached(const HttpCookie &cookie) const {
	++TheTotalCount;
	static bool reported(false);
	if (!reported) {
		printMessage("the first cookie cached", cookie);
		reported = true;
	}
}

void HttpCookies::reportUpdated(const HttpCookie &cookie) const {
	static bool reported(false);
	if (!reported) {
		printMessage("the first cookie updated", cookie);
		reported = true;
	}
	++TheUpdatedCount;
}

void HttpCookies::reportPurged(const HttpCookie &cookie) const {
	if (cookie.expired()) {
		static bool reported(false);
		if (!reported) {
			printMessage("the first stale cookie evicted", cookie);
			reported = true;
		}
		++ThePurgedStaleCount;
	} else {
		static bool reported(false);
		if (!reported) {
			printMessage("the first fresh cookie evicted", cookie);
			reported = true;
		}
		++ThePurgedFreshCount;
	}
	if (Should(TheTotalCount > 0))
		--TheTotalCount;
}

void HttpCookies::printMessage(const char *const message, const HttpCookie &cookie) const {
	Comment(7)
		<< "fyi: " << message
		<< ", cookie: " << cookie.data()
		<< ", number of cookies: "
		<< theCookies.size() << '/' << TheTotalCount
		<< endl << endc
		;
}
