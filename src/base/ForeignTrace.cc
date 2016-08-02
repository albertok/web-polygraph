
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include <fstream>
#include <ctype.h>

#include "xstd/NetAddr.h"
#include "base/AddrParsers.h"
#include "base/ForeignTrace.h"


ForeignTrace::ForeignTrace(): doIgnoreBad(true) {
}

void ForeignTrace::configure(const String &aName, bool ignoreBad) {
	theName = aName;
	doIgnoreBad = ignoreBad;
}


int ForeignTrace::gatherUrls(Array<String*> &urls) const {
	ifstream is(theName.cstr());
	if (open(is)) {
		while (String *url = getUrl(is)) {
			urls.append(url);
			theMemSize += url->len();     // content
			theMemSize += SizeOf(String);  // overhead
			theMemSize += SizeOf(String*); // overhead
		}
		close(urls.count());
	}
	return urls.count();
}

int ForeignTrace::gatherHosts(Array<NetAddr*> &hosts) const {
	ifstream is(theName.cstr());
	if (open(is)) {
		while (String *url = getUrl(is)) {
			// extract host from the URL
			NetAddr host;
			const char *urlB = url->cstr();
			const char *urlE = urlB + url->len();
			if (const char *hostEnd = SkipHostInUri(urlB, urlE, host)) {
				hosts.append(new NetAddr(host));
				theMemSize += hostEnd - urlB;   // content
				theMemSize += SizeOf(NetAddr);  // overhead
				theMemSize += SizeOf(NetAddr*); // overhead
			}
			delete url;
		}
		close(hosts.count());
	}
	return hosts.count();
}

String *ForeignTrace::getUrl(istream &is) const {
	char line[16*1024];
	while (is.good() && is.getline(line, sizeof(line))) {

		// delete comments
		if (char *comment = strchr(line, '#'))
			*comment = '\0';

		// find first URL on the line
		const char *urlB;
		if ((urlB = strstr(line, "http://")) ||
			(urlB = strstr(line, "https://")) ||
			(urlB = strstr(line, "ftp://"))) {
			// find URL end (white space or eol)
			const char *urlE = urlB;
			while (*urlE && !isspace(*urlE))
				++urlE;

			String *url = new String;
			url->append(urlB, urlE - urlB);
			++theEntryCount;
			return url;
		}

		// skip leading spaces to avoid warning about empty lines
		urlB = line;
		while (*urlB && isspace(*urlB))
			++urlB;

		if (*urlB && !doIgnoreBad) {
			cerr << here <<
				"error: all trace URLs must follow " <<
				"http|https|ftp://host/path format; " << endl <<
				"\tfound: " << urlB << endl;
		}
	}

	return 0;
}

bool ForeignTrace::open(istream &is) const {
	theMemSize = 0;
	theEntryCount = 0;
	if (!is) {
		cerr << "failed to open '" << theName << "' trace for reading: " <<
			Error::Last() << endl;
		return false;
	}
	return true;
}

void ForeignTrace::close(const int goodCount) const {
	if (goodCount > 0)
		clog << "fyi: loaded trace from ";
	else
		clog << "warning: empty trace in ";

	clog << "'" << theName << "': " <<
		"used " << goodCount << " entries out of " << theEntryCount << ", " <<
		"spent at least " << theMemSize << " bytes" <<
		endl;
}
