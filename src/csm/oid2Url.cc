
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/NetAddr.h"
#include "base/RndPermut.h"
#include "base/AddrParsers.h"
#include "base/ObjId.h"
#include "runtime/httpHdrs.h"
#include "runtime/HostMap.h"
#include "runtime/LogComment.h"
#include "csm/ContentCfg.h"
#include "csm/ContentMgr.h"
#include "csm/ContentSel.h"
#include "csm/oid2Url.h"
#include "csm/RamFiles.h"

static
void oidDumpHost(const ObjId &oid, const NetAddr &host, const bool forcePortPrint, ostream &os) {
	if (oid.prefix() > 0)
		os << 'w' << setw(16) << hex << setfill('0') << oid.prefix() << setw(0) << dec;
	bool haveDefaultPort = false;
	if (oid.secure())
		haveDefaultPort = host.port() == 443;
	else
	if (oid.scheme() == Agent::pHTTP)
		haveDefaultPort = host.port() == 80;
	else
		haveDefaultPort = host.port() == 21;
	if (forcePortPrint || !haveDefaultPort)
		os << host; // prints host:port
	else
		os << host.addrA(); // prints the host name or IP only
}

NetAddr Oid2UrlHost(const ObjId &oid) {
	if (oid.foreignUrl()) {
		const char *furi = oid.foreignUrl().data();
		NetAddr host;
		if (SkipHostInUri(furi, furi+oid.foreignUrl().len(), host))
			return host;

		// XXX: We should use ReportError() here but we do not know the log
		// category, and the API sucks in libruntime, which cdb cannot use.
		Comment(1) << "error: cannot extract host name from a foreign URL: " <<
			oid.foreignUrl() << endc;
		return NetAddr();
	} else
	if (const HostCfg *const host = TheHostMap->at(oid.viserv())) {
		if (host->theAddr.isDynamicName()) {
			Assert(oid.prefix() > 0);
			// XXX: Poor performance
			// 18 chars for 'w', 16 digit hex prefix and terminating zero
			char buf[18 + host->theAddr.addrA().len()];
			ofixedstream os(buf, sizeof(buf));
			NetAddr addr(host->theAddr.addrA(), -1); // remove port number
			oidDumpHost(oid, addr, true, os);
			addr.addr(String(buf, os.tellp()));
			addr.port(host->theAddr.port());
			return addr;
		} else
			return host->theAddr;
	}

	Should(false);
	return NetAddr();
}

ostream &Oid2UrlHost(const ObjId &oid, const bool forcePortPrint, ostream &os) {
	if (oid.foreignUrl()) {
		const NetAddr host = Oid2UrlHost(oid);
		if (host)
			oidDumpHost(oid, host, forcePortPrint, os);
		else
			os << '?';
	} else
	if (const HostCfg *const host = TheHostMap->at(oid.viserv()))
		oidDumpHost(oid, host->theAddr, forcePortPrint, os);
	else
		os << oid.viserv();

	return os;
}

ostream &Oid2UrlPath(const ObjId &oid, ostream &os) {
	if (oid.foreignUrl()) {
		const char *p = 0;
		if ((p = oid.foreignUrl().chr(':'))) { // proto
			while (*++p == '/');
		} else {
			p = oid.foreignUrl().cstr();
		}
		if (const char *path = strchr(p, '/'))
			os << path;
		else
			os << '/';
		return os;
	}

	const ContentCfg *ccfg = 0;
	if (oid.type() > 0) {
		ccfg = TheContentMgr.get(oid.type());
		Assert(ccfg);
	}

	if (ccfg && ccfg->ramFiles())
		return os << ccfg->ramFile(oid).name;

	os << '/';

	if (ccfg) {
		const int seed = GlbPermut(oid.hash(), rndContentPfx);
		if (const String &pfx = ccfg->url_pfx(seed))
			os << pfx;
	}

	os << 'w' << oid.world() << '/';

	// use non-default formatting to save space and reduce length variation
	{
		const ios_fmtflags flags = os.flags();
		const char fill = os.fill('0');
		os << hex << setfill('0');
		os << 't' << setw(2) << oid.type() << '/';
		os << '_' << setw(16) << oid.name();
		os.fill(fill);
		os.flags(flags);
	}

	if (ccfg) {
		const int seed = GlbPermut(oid.hash(), rndContentExt);
		if (const String &ext = ccfg->url_ext(seed))
			os << ext;
	}
	return os;
}

ostream &Oid2Url(const ObjId &oid, ostream &os) {
	if (oid.foreignUrl())
		return os << oid.foreignUrl();

	if (oid.secure())
		os << "https://";
	else
		os << (oid.scheme() == Agent::pHTTP ? "http://" : "ftp://");

	Oid2UrlHost(oid, false, os);
	Oid2UrlPath(oid, os);
	return os;
}

bool OidImpliesMarkup(const ObjId &oid, const ContentCfg *cfg) {
	String urlSfx = oid.foreignUrl();
	if (!urlSfx) {
		Assert(cfg); // add code to compute if needed
		const int seed = GlbPermut(oid.hash(), rndContentExt);
		urlSfx = cfg->url_ext(seed);
	}

	return urlSfx && (urlSfx.last() == '/' || urlSfx.caseEndsWith("ml", 2));
}

// Computes URL prefix ending with the last slash, excluding that slash.
// Useful for directory-based authentication.
String Oid2AuthPath(const ObjId &oid) {
	static char buf[16*1024];
	ofixedstream os(buf, sizeof(buf));
	Oid2UrlPath(oid, os);
	os << ends;
	buf[sizeof(buf) - 1] = '\0';
	const char *const pos(strrchr(buf, '/'));
	Assert(pos);
	return String(buf, pos - buf);
}
