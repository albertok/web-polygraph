
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/h/sstream.h"

#include "base/macros.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/PglNetAddrRangeSym.h"
#include "pgl/PglArraySym.h"
#include "pgl/IpsToNames.h"
#include "pgl/PglSemx.h"


static
String ipToNameSimple(const NetAddr &ip, const String &dname) {
	char buf[1024];
	ofixedstream os(buf, sizeof(buf));
	os << "w[" << 1000 + ip.roctet(0)
		<< "].h[" << 1000 + ip.roctet(1)
		<< "]o[" << 1000 + ip.roctet(2) 
		<< "]s[" << 1000 + ip.roctet(3)
		<< "]." << dname;
	if (ip.port() >= 0)
		os << ':' << ip.port();
	os << ends;
	return buf;
}

static
String ipToNameWithMacros(const TokenLoc &loc, const NetAddr &ip, const String &dname) {
	char buf[1024];
	ofixedstream os(buf, sizeof(buf));
	for (int i = 0; i < ip.addrN().len(); ++i) {
		if (i)
			os << '-';
		os << '[' << ip.octet(i) << ']';
	}
	os << ends;

	static const String ip_macro("${dashed_ip}");
	String name = ExpandMacro(dname, ip_macro, buf);

	static const String worker_id_macro("${worker}");
	name = ExpandMacro(name, worker_id_macro, PglSemx::WorkerIdStr());

	static const String port_macro("${port}");
	if (ip.port() >= 0) {
		os.seekp(0); // reset stream
		os << ip.port() << ends;

		name = ExpandMacro(name, port_macro, buf);
	} else if (name.str(port_macro)) {
		cerr << loc << "IpsToNames() template '" << dname << "' has "
			"${port} macro, but address '" << ip << "' has no port"
			<< endl << xexit;
	}
	return name;
}

ArraySym *IpsToNames(const ContainerSym &ips, const String &dname) {

	Array<PglNetAddrRange*> ranges;
	const int ipCount = ips.count();
	const bool haveMacros = dname.find('$') != String::npos;
	for (int i = 0; i < ipCount; ++i) {
		const NetAddrSym &ipSym = (const NetAddrSym&)ips.item(i)->cast(NetAddrSym::TheType);
		const NetAddr &ip = ipSym.val();
		const String &name = haveMacros ?
			ipToNameWithMacros(ips.loc(), ip, dname) :
			ipToNameSimple(ip, dname);
		PglNetAddrRange r;
		Assert(r.parse(name));

		// if cannot merge r with the last range, add new range
		if (!ranges.count() || !ranges.last()->canMerge(r))
			ranges.append(new PglNetAddrRange);

		Assert(ranges.count());
		ranges.last()->merge(r);
	}

	// XXX: unify with same code in VerFourAs!
	// merge ranges if possible
	bool merged = true;
	do {
		merged = false;
		for (int left = 0, right = 1; !merged && right < ranges.count(); ++right) {
			PglNetAddrRange *&r = ranges[right];
			if (!r) // deleted by earlier merges
				continue;
			if (ranges[left] && ranges[left]->canMerge(*r)) {
				ranges[left]->merge(*r);
				delete r;
				r = 0;
				merged = true;
			} else {
				left = right;
			}
		}
	} while (merged);

	// convert an array of ranges into ArraySym
	ArraySym *names = new ArraySym(NetAddrSym::TheType);
	{for (int i = 0; i < ranges.count(); ++i) {
		if (!ranges[i])
			continue;
		NetAddrRangeSym rs;
		rs.range(ranges[i]);
		names->add(rs);
	}}

	return names;
}
