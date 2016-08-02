
/* Web Polygraph       http://www.web-polygraph.org/
 * (C) 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/Rnd.h"
#include "base/RndPermut.h"
#include "pgl/MimeHeaderSym.h"
#include "runtime/httpHdrs.h"
#include "runtime/LogComment.h"
#include "runtime/MimeHeadersCfg.h"

#include "runtime/HttpPrinter.h"


HttpPrinter::HttpPrinter(char_type *const buffer, const std::streamsize size):
	ofixedstream(buffer, size) {
}

bool HttpPrinter::putHeader(const String &header, const bool isUserConfigured) {
	if (!isUserConfigured && !MimeHeadersCfg::Clashes()) {
		*this << header;
		return true;
	}

	const int end = header.find(':');
	const String name = header(0, end);
	const bool added = theHeaders.insert(name).second;
	if (added)
		*this << header;
	else {
		static const String msg = isUserConfigured ? "user-configured" :
			"user-configured and Polygraph-generated";
		Comment(5) << "warning: conflicting " << msg << " HTTP header '"
			<< name << '\'' << std::endl << endc;
	}
	return added;
}

void HttpPrinter::putHeaders(const MimeHeadersCfg *const cfg) {
	if (cfg)
		cfg->selectHeaders(*this);
}

void HttpPrinter::visit(const SynSym &item) {
	const MimeHeaderSym &sym =
		(const MimeHeaderSym&)item.cast(MimeHeaderSym::TheType);
	const String *const name = sym.name();
	Assert(name && *name);
	if (putHeader(*name, true)) {
		*this <<  ": ";
		if (const String *const value = sym.value())
			*this << *value;
		*this << crlf;
	}
}
