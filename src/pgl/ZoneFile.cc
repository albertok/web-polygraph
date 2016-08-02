
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2014 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/ZoneFile.h"

#include <cstdlib>

#if LDNS_ENABLED
#	if HAVE_BROKEN_LDNS_H
		extern "C" {
#	endif
#	if HAVE_CLASS_IN_LDNS
#		define class classC
#	endif
#	include <ldns/ldns.h>
#	if HAVE_CLASS_IN_LDNS
#		undef class
#	endif
#	if HAVE_BROKEN_LDNS_H
		}
#	endif

// zone file reader
class ZoneFile {
	public:
		bool load(const String file);
		String *findNameByIp(const InAddress &addr);

	private:
		// Domain names and/or IPs loaded from the zone file.
		// The items are added in the reverse zone file order, for now.
		typedef Array<InAddress*> Ips;
		typedef Array<String*> Names;
		Names names; // domain names
		Ips ips; // IP addresses
};

bool ZoneFile::load(const String file) {
	FILE *fp = fopen(file.cstr(), "r");
	if (!fp) {
		cerr << "Error opening zone file " << file << ": " << Error::Last() << endl;
		return false;
	}

	ldns_zone *z = 0;
	int badLine = -1;
	ldns_status status = ldns_zone_new_frm_fp_l(&z, fp, 0, 0, LDNS_RR_CLASS_IN, &badLine);
	fclose(fp);

	if (status != LDNS_STATUS_OK) {
		cerr << file << ':' << badLine << ": " <<
			ldns_get_errorstr_by_id(status) << endl;
		return false;
	}

	char raw_buf[10240];
	char owner[10240];
	ldns_rr *cur_rr;

	ldns_buffer *lbuf = ldns_buffer_new(sizeof(raw_buf));
	Assert(lbuf);

	while ((cur_rr = ldns_rr_list_pop_rr(ldns_zone_rrs(z)))) {
		const ldns_rr_type cur_rr_type = ldns_rr_get_type(cur_rr);
		if (cur_rr_type == LDNS_RR_TYPE_A || cur_rr_type == LDNS_RR_TYPE_AAAA) {
			ldns_rdf *rdf = ldns_rr_owner(cur_rr);
			Assert(rdf != 0);
			ldns_buffer_clear(lbuf);
			status = ldns_rdf2buffer_str_dname(lbuf, rdf);
			Assert(status == LDNS_STATUS_OK);
			size_t raw_count = ldns_buffer_position(lbuf);
			Assert(raw_count < sizeof(owner));
			ldns_buffer_read_at(lbuf, 0, &owner, raw_count);
			owner[raw_count] = '\0'; /* terminate */

			String name(owner);
			Assert(name);
			// Chop the terminating dot, if any. TODO: This trimming is out
			// of scope here; the dot may be useful in some DNS contexts.
			if (name.endsWith(".", 1))
				name = name(0, name.len()-1);

			ldns_buffer_clear(lbuf);

			if (name && name[0] != '*') {
				for (size_t i = 0; i < ldns_rr_rd_count(cur_rr); ++i) {
					status = ldns_rdf2buffer_wire(lbuf, ldns_rr_rdf(cur_rr, i));
					Assert(status == LDNS_STATUS_OK);
					raw_count = ldns_buffer_position(lbuf);
					ldns_buffer_read_at(lbuf, 0, &raw_buf, raw_count);
					InAddress *ipAddr = 0;
					if (cur_rr_type == LDNS_RR_TYPE_A) {
						struct in_addr ip4;
						memcpy(&ip4, raw_buf, sizeof(ip4));
						ipAddr = new InAddress(ip4);
					} else {
						struct in6_addr ip6;
						memcpy(&ip6, raw_buf, sizeof(ip6));
						ipAddr = new InAddress(ip6);
					}
					Assert(ipAddr);

					ips.push(ipAddr);
					names.push(new String(name));
				}
			}
		}
		ldns_rr_free(cur_rr);
	}

	ldns_buffer_free(lbuf);
	ldns_rr_list_free(ldns_zone_rrs(z));
	return true;
}

String *ZoneFile::findNameByIp(const InAddress &ip) {
	// TODO: build an index to avoid linear search?
	for (int i = ips.count() - 1; i >= 0; --i) {
		if (*ips[i] == ip)
			return names[i];
	}
	return 0;
}

#endif /* LDNS_ENABLED */


ArraySym *ZoneDomains(const TokenLoc &loc, const String &fname, const ContainerSym &ips) {
#if LDNS_ENABLED
	ZoneFile zone;
	if (!zone.load(fname))
		cerr << loc << "error: cannot load zone from " << fname << endl << xexit;

	ArraySym *newDomains = new ArraySym(NetAddrSym::TheType);

	for (int i = 0; i < ips.count(); ++i) {
		const NetAddrSym &ipSym = (const NetAddrSym&)ips.item(i)->cast(NetAddrSym::TheType);
		const NetAddr &ip = ipSym.val();

		if (const String *name = zone.findNameByIp(ip.addrN())) {
			NetAddr addr(*name, ip.port());
			NetAddrSym addrSym;
			addrSym.val(addr);
			newDomains->add(addrSym);
		} else {
			const char *clas = ip.addrN().family() == AF_INET ? "A" : "AAAA";
			cerr << loc << "error: no IN " << clas << " record with " <<
				ip.addrN().image() << " IP in the zone file: " << fname << endl << xexit;
		}
	}

	return newDomains;
#else
	cerr << loc << "error: PGL zoneDomains() function requires libldns support" << endl << xexit;
	return 0;
#endif /* LDNS_ENABLED */
}
