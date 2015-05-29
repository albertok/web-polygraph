
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/String.h"
#include "logextractors/matchAndPrint.h"

// format:
//		<name>: <body>\n
// or
//		<name>:\n
//		<multiline-body>
//		\n


typedef void (*MatchAndPrintEntry)(ostream &os, const char *start, const char *body_start, const char *end);

static
bool NameStart(const char *pos, const char *name, const char *&start) {
	start = strstr(pos, name);
	return start != 0;
}

static
int MatchAndPrintDriver(ostream &os, const char *buf, const Array<String*> &names, MatchAndPrintEntry mape) {
	int matchCount = 0;

	for (int i = 0; i < names.count(); ++i) {
		const String name = *names[i];
		int nameMatchCount = 0;
		const char *pos = buf;
		const char *start;
		while (NameStart(pos, name.cstr(), start)) {
			// check the start of the name
			if (start > pos && !isspace(start[-1])) {
				pos = start+1;
				continue;
			}

			// check the end of the name
			pos = start + name.len();
			if (!(*pos == '.' || *pos == ':'))
				continue;

			// seek to the end of the name
			pos = strchr(pos, ':');
			if (!pos)
				break;
			pos++;
			const char *body_start = pos;

			// find the end of the object
			const char *end = strstr(pos, "\n");
			if (end) {
				if (end == pos)
					end = strstr(end, "\n\n");
				if (end) {
					mape(os, start, body_start, end);
					nameMatchCount++;
					pos = end;
				}
			}
		}
		if (!nameMatchCount)
			cerr << "warning: found no objects named `" << name << "'" << endl;
		else
			matchCount++;
	}

	return matchCount;
}

static
void mapeWithTags(ostream &os, const char *start, const char *, const char *end) {
	os.write(start, end-start);
	os << endl;
}

static
void mapeBodies(ostream &os, const char *, const char *body_start, const char *end) {
	while (body_start < end && isspace(*body_start)) ++body_start;
	os.write(body_start, end-body_start);
	os << ' ';
}

static
void mapeTags(ostream &os, const char *start, const char *body_start, const char *) {
	os.write(start, body_start-start - 1);
	os << ' ';
}


int MatchAndPrint(ostream &os, const char *buf, const Array<String*> &names) {
	if (!names.count()) {
		os << buf;
		return -1;
	}
	return MatchAndPrintDriver(os, buf, names, &mapeWithTags);
}

int MatchAndPrintBodies(ostream &os, const char *buf, const Array<String*> &names) {
	const int res = MatchAndPrintDriver(os, buf, names, &mapeBodies);
	os << endl;
	return res;
}

int MatchAndPrintHeader(ostream &os, const char *buf, const Array<String*> &names) {
	os << '#';
	const int res = MatchAndPrintDriver(os, buf, names, &mapeTags);
	os << endl;
	return res;
}

