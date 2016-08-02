
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_HTTPCOOKIES_H
#define POLYGRAPH__RUNTIME_HTTPCOOKIES_H

#include "xstd/String.h"
#include "runtime/Farm.h"

#include <list>
#include <map>

// manages a single HTTP Cookie
class HttpCookie {
	public:
		const String &name() const { return theName; }
		const String &data() const { return theData; }

		void reset();
		void set(const String &aData, const int nameEnd);
		bool expired() const;
		bool discardable() const;

		static void Configure();
		// cookies are destroyed with Put()
		static void Put(HttpCookie *const cookie);
		// cookies are created with Parse()
		static HttpCookie *Parse(const char *buf, const char *eoh);

	private:
		static bool ParseParameter(const char *const buf, const char *const end, HttpCookie &cookie);

		// protected to force Farm use
		HttpCookie();
		~HttpCookie() {};

		HttpCookie(const HttpCookie &); // undefined
		HttpCookie &operator =(const HttpCookie &); // undefined

		void discard() { isDiscardable = true; }
		void expires(const Time &data) { theExpires = data; }
		void maxAge(const int delta);

		String theName;
		String theData;
		Time theExpires;
		bool isDiscardable;

		friend class ObjFarm<HttpCookie>;
		static ObjFarm<HttpCookie> TheCookieFarm;
};

// server cookies storage
class HttpCookies {
	public:
		HttpCookies();
		~HttpCookies();

		void keepLimit(const unsigned limit);

		void add(HttpCookie *const cookie);

		int count() const;

		// first/next fresh cookie in undefined order
		const HttpCookie *first();
		const HttpCookie *next();

		void purgeDiscardable();

		static Counter TheTotalCount;
		static Counter ThePurgedFreshCount;
		static Counter ThePurgedStaleCount;
		static Counter TheUpdatedCount;

	private:
		typedef std::list<HttpCookie*> Cache;

		void purgeAt(const Cache::iterator &i, const bool purgeName = true, const bool doReport = true);
		const HttpCookie *sync();

		void reportCached(const HttpCookie &cookie) const;
		void reportUpdated(const HttpCookie &cookie) const;
		void reportPurged(const HttpCookie &cookie) const;
		void printMessage(const char *const message, const HttpCookie &cookie) const;

		unsigned theLimit; // maximum number of cookies to store
		Cache theCookies;
		Cache::iterator theIter; // current cookie pointer, used for getting cookies
		typedef std::map<String, Cache::iterator> NameMap;
		NameMap theNameMap; // name:cookie in cache
};

#endif
