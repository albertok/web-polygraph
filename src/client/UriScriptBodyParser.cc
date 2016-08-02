
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>

#include "xstd/Rnd.h"
#include "xstd/gadgets.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "client/CltXact.h"
#include "client/CltOpts.h"
#include "client/ParseBuffer.h"
#include "client/UriScriptBodyParser.h"

// commonly used RE patterns 
static const String ptnConst = "'([^']*)'";
static const String ptnName = "([[:alnum:]_]+)";
static const String ptnSpace = "[[:space:]]*";
static const String ptnEq = ptnSpace + "=" + ptnSpace;

RegEx UriScriptBodyParser::TheWre;
RegEx UriScriptBodyParser::TheXre;
RegEx UriScriptBodyParser::TheYre;
RegEx UriScriptBodyParser::TheZre;

BodyParserFarmT<UriScriptBodyParser> UriScriptBodyParser::TheParsers;

BodyParser *UriScriptBodyParser::GetOne(CltXact *anOwner, const CltCfg *aCfg) {
	if (!TheParsers.capacity())
		TheParsers.limit(1024);

	UriScriptBodyParser *parser = TheParsers.getTyped();
	parser->configure(anOwner, aCfg);
	return parser;
}

UriScriptBodyParser::UriScriptBodyParser() {
	// configure static REs if needed
	if (!TheXre.configured()) {
		ConfigureScriptVarRe(TheWre, "w");
		ConfigureScriptVarRe(TheXre, "x");
		ConfigureScriptVarRe(TheYre, "y");
		ConfigureScriptVarRe(TheZre, "z");
	}

	resetSelf();
}

void UriScriptBodyParser::reset() {
	resetSelf();
	BodyParser::reset();
}

void UriScriptBodyParser::resetSelf() {
	theCfg = 0;
	theScriptBeg = theScriptEnd = 0;
	theState = stNone;
}

void UriScriptBodyParser::configure(CltXact *anOwner, const CltCfg *aCfg) {
	BodyParser::configure(anOwner);
	Check(!theCfg && aCfg);
	theCfg = aCfg;
}

BodyParserFarm &UriScriptBodyParser::farm() const {
	return TheParsers;
}

Size UriScriptBodyParser::parse(const ParseBuffer &buf) {
	if (theState == stNone)
		theState = stOpen;

	if (theState == stOpen)
		parseOpen(buf);

	if (theState == stClose)
		parseClose(buf);

	if (theState == stBody)
		parseBody();

	if (theState == stDone)
		return parseAny(buf);

	return 0; // need more data
}

void UriScriptBodyParser::parseOpen(const ParseBuffer &buf) {
	static const String pfx = "<script";
	if (buf.size() < pfx.len())
		return; // need more data

	if (strncmp(buf.data(), pfx.cstr(), pfx.len()) == 0)
		theState = stClose;
	else
		theState = stDone;
}

void UriScriptBodyParser::parseClose(const ParseBuffer &buf) {
	static const String sfx = "</script";
	if (buf.size() < sfx.len())
		return; // need more data
	
	const char *bufEnd = buf.data() + buf.size();
	theScriptBeg = buf.data();
	theScriptEnd = StrBoundStr(theScriptBeg, sfx.cstr(), bufEnd);
	if (theScriptEnd)
		theState = stBody;
}

void UriScriptBodyParser::parseBody() {
	if (Should(theScriptBeg) && Should(theScriptEnd))
		parseVars();
	theState = stDone;
}

void UriScriptBodyParser::parseVars() {
	String w, x;
	if (parseVar("w", TheWre, w) && parseVar("x", TheXre, x)) {
		// XXX: kludge to do 95%/5% split
		static RndGen rng;
		const bool formerFrameless = rng.event(0.95);
		forwardUri(x + (formerFrameless ? "/1-": "/2-") + w);
	}

	String y;
	if (parseVar("y", TheYre, y))
		forwardUri(y);

	String z;
	if (parseVar(0, TheZre, z))
		forwardUri(z);
}

bool UriScriptBodyParser::parseVar(const char *name, const RegEx &re, String &value) {
	static RegEx::Matches matches(1+1);
	static const int expectedCount = matches.capacity();
	matches.reset();
	matches.append(RegEx::StartEnd(0, theScriptEnd - theScriptBeg));
	if (re.match(theScriptBeg, matches, RegEx::reStartEnd) && 
		Should(matches.count() == expectedCount)) {
		const RegEx::Match &m = matches[1];
		if (m.rm_so >= 0 && m.rm_eo > m.rm_so) {
			value = String(theScriptBeg + m.rm_so,  m.rm_eo - m.rm_so);
			return true;
		}
	}

	if (name) { // client asked to report errors
		const Error &err = errForeignTag;
		if (!TheCltOpts.ignoreBadContTags && ReportError(err)) {
			dumpContext(Comment << "undefined or malformed variable '" <<
				name << "' in the following emdedded script",
				theScriptBeg, theScriptEnd - theScriptBeg) << endc;
		}
	}
	return false;
}
	
void UriScriptBodyParser::forwardUri(const String &uri) {
	Error err;
	const char *parsep = uri.cstr();
	ReqHdr hdr;
	if (hdr.parseUri(parsep, parsep+uri.len(), hdr.theUri))
		err = theOwner->noteEmbedded(hdr);
	else
		err = errForeignTag;

	if (err) {
		if (!TheCltOpts.ignoreBadContTags && ReportError(err)) {
			dumpContext(Comment << "unparseable recreated URL: ",
				uri.data(), uri.len()) << endc;
		}
	} else {
		static int reportCount = 0;
		if (!reportCount++) {
			dumpContext(Comment(1) << "fyi: first URL extracted " <<
				"from a script: ", uri.data(), uri.len()) << endc;
		}
	}
}

Size UriScriptBodyParser::parseAny(const ParseBuffer &buf) {
	const Size parsedSize = buf.size();
	theOwner->noteContent(buf);
	return parsedSize;
}

void UriScriptBodyParser::noteLeftovers(const ParseBuffer &leftovers) {
	Should(theState == stOpen || theState == stClose);
	if (theState == stClose && ReportError(errContentLeftovers)) {
		const char *what = theState == stOpen ? "beginning" : "end";
		dumpContext(Comment << "unable to locate the " << what <<
			" of a URI-setting embedded <script> in the remaining " <<
			leftovers.size() << " of content leftovers near " << endl,
			leftovers.data(), leftovers.size()) << endc;
	}
	theOwner->noteContent(leftovers);
}

void UriScriptBodyParser::noteOverflow(const ParseBuffer &buf) {
	if (ReportError(errHugeContentToken)) {
		dumpContext(Comment << "huge URI-setting <script> context near ",
		buf.data(), buf.size()) << endc;
	}
	theOwner->noteContent(buf);
}


void UriScriptBodyParser::ConfigureScriptVarRe(RegEx &re, const String &name) {
	String pattern;
	pattern += name;
	pattern += ptnEq;
	pattern += ptnSpace;
	pattern += ptnConst;

	re.configure(name + " RE", pattern);
	Should(re);
}
