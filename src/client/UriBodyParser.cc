
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>

#include "xstd/gadgets.h"
#include "base/StatIntvlRec.h"
#include "runtime/LogComment.h"
#include "runtime/ErrorMgr.h"
#include "runtime/polyErrors.h"
#include "client/CltXact.h"
#include "client/CltOpts.h"
//#include "client/CltCfg.h"
#include "client/ParseBuffer.h"
#include "client/UriBodyParser.h"


BodyParserFarmT<UriBodyParser> UriBodyParser::TheParsers;


BodyParser *UriBodyParser::GetOne(CltXact *anOwner, const CltCfg *aCfg) {
	if (!TheParsers.capacity())
		TheParsers.limit(1024);

	UriBodyParser *parser = TheParsers.getTyped();
	parser->configure(anOwner, aCfg);
	return parser;
}

UriBodyParser::UriBodyParser(): theCfg(0) {
}

void UriBodyParser::reset() {
	resetSelf();
	BodyParser::reset();
}

void UriBodyParser::resetSelf() {
	theCfg = 0;
}

void UriBodyParser::configure(CltXact *anOwner, const CltCfg *aCfg) {
	BodyParser::configure(anOwner);
	Check(!theCfg && aCfg);
	theCfg = aCfg;
}

BodyParserFarm &UriBodyParser::farm() const {
	return TheParsers;
}

static
bool isUriChar(const char c) {
	return isalnum(c) ||
		c == ';' || c == '?' || c == ':' || c == '@' || c == '%' || c == '#' ||
		c == '&' || c == '=' || c == '-' || c == '+' || c == '$' || c == ',' ||
		c == '/' || c == '.' || c == '_' || c == '!' || c == '~' || c == '*';
}

Size UriBodyParser::parse(const ParseBuffer &buf) {
	if (buf.empty())
		return 0;

	// limit URIs to HTTP URLs for now
	const String pfx = "http://";

	const char *bufEnd = buf.data() + buf.size();
	const char *parsedBeg = buf.data();
	const char *uriBeg = 0;

	// XXX: if pfx is split across two parse calls, we will miss that URL
	while ((uriBeg = StrBoundStr(parsedBeg, pfx.cstr(), bufEnd))) {
		parsedBeg = uriBeg;
		const char *uriBody = uriBeg + pfx.len();
		const char *uriEnd = 0;
		for (const char *p = uriBody; !uriEnd && p < bufEnd; ++p) {
			if (!isUriChar(*p))
				uriEnd = p;
		}

		if (uriEnd) {
			parsedBeg = uriEnd;

			// delete # fragment, if any
			if (const char *uriFrag = StrBoundChr(uriBody, '#', uriEnd))
				uriEnd= uriFrag;

			Error err;
			ReqHdr hdr;
			const char *parsep = uriBeg;
			if (hdr.parseUri(parsep, uriEnd, hdr.theUri))
				err = theOwner->noteEmbedded(hdr);
			else
				err = errForeignTag;

			if (err && !TheCltOpts.ignoreBadContTags && ReportError(err)) {
				dumpContext(Comment << "unparseable URL: ", uriBeg, 
					uriBeg-uriEnd) << endc;
			}
		} else {
			// need more data
			break;
		}
	}

	if (!uriBeg) // do not need more data
		parsedBeg = bufEnd;

	const Size parsedSize = parsedBeg - buf.data();
	theOwner->noteContent(buf.head(parsedSize));
	return parsedSize;
}

void UriBodyParser::noteLeftovers(const ParseBuffer &leftovers) {
	if (ReportError(errContentLeftovers)) {
		dumpContext(Comment << "unterminated URI tag near ", 
			leftovers.data(), leftovers.size()) << endc;
	}
	theOwner->noteContent(leftovers);
}

void UriBodyParser::noteOverflow(const ParseBuffer &buf) {
	if (ReportError(errHugeContentToken)) {
		dumpContext(Comment << "huge URI near ",
			buf.data(), buf.size()) << endc;
	}
	theOwner->noteContent(buf);
}
