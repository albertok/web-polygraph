
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "xstd/h/iostream.h"
#include "xstd/h/sstream.h"
#include <fstream>
#include "xstd/h/iomanip.h"

#include "xstd/gadgets.h"
#include "base/CmdLine.h"
#include "base/opts.h"
#include "base/polyOpts.h"
#include "pgl/PglArraySym.h"
#include "pgl/PglNetAddrSym.h"
#include "pgl/PglNetAddrRange.h"
#include "pgl/PglNetAddrRangeSym.h"
#include "pgl/AddrMapSym.h"
#include "pgl/PglPp.h"
#include "pgl/PglStaticSemx.h"


class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,    "help",          "list of options"),
			theVersOpt(this,    "version",       "package version info"),
			theCfgName(this,    "config <filename>",  "PGL configuration"),
			theCfgDirs(this,    "cfg_dirs <dirs>",  "directories for PGL #includes"),
			theZoneName(this,   "zone <str>",    "zone name"),
			theAddrStr(this,    "addresses <str>","IP addresses"),
			theNameStr(this,    "names <str>",   "domain names")
			{}

		virtual bool validate() const;

		//virtual ostream &printAnonym(ostream &os) const;
		//virtual bool parseAnonym(const Array<const char *> &opts);
		//virtual bool canParseAnonym() const { return true; }

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theCfgName;
		StrArrOpt theCfgDirs;
		StrOpt theZoneName;
		StrOpt theAddrStr;
		StrOpt theNameStr;
};

static String TheZoneName;
static String TheZoneOrRoot;
static String TheNameServerName;
static String TheNameServerIp;
static ArraySym TheAddrs("addr");
static ArraySym TheNames("addr");
static MyOpts TheOpts;


/* MyOpt */

bool MyOpts::validate() const {
	if (theCfgName) {
		if (theZoneName)
			cerr << "--config is mutually exclusive with --zone" << endl;
		else
		if (theAddrStr || theNameStr)
			cerr << "--config is mutually exclusive with --addresses and --names" << endl;
		else
			return true;
	} else {
		if (!theZoneName)
			cerr << "must specify the zone name" << endl;
		else
		if (!theAddrStr || !theNameStr)
			cerr << "must specify addresses and domain names" << endl;
		else
			return true;
	}
	return false;
}

static
void configureLogs(int prec) {
	configureStream(cout, prec);
	configureStream(cerr, prec);
	configureStream(clog, prec);
}

static
void configureViaPgl() {
	TheOpts.theCfgDirs.copy(PglPp::TheDirs);
	PglStaticSemx::Interpret(TheOpts.theCfgName);

	if (PglStaticSemx::TheAddrMapsToUse.count() != 1) {
		cerr << TheOpts.theCfgName << ": must use() exactly one address map" <<
			", but " << PglStaticSemx::TheAddrMapsToUse.count() << " maps " <<
			"were used" << endl << xexit;
	}

	const AddrMapSym *ams = PglStaticSemx::TheAddrMapsToUse.last();
	
	TheZoneName = ams->zone();
	TheAddrs.append(*ams->addressesSym());
	TheNames.append(*ams->namesSym());

	if (!TheZoneName) {
		cerr << ams->loc() << "address map must specify zone name" 
			<< endl << xexit;
	}
	if (!TheAddrs.count()) {
		cerr << ams->loc() << "address map must specify at least one IP addresses" 
			<< endl << xexit;
	}
	if (!TheNames.count()) {
		cerr << ams->loc() << "address map must specify at least one domain name" 
			<< endl << xexit;
	}
}

static
void configureViaCmdLine() {
	PglNetAddrRange addrs;
	PglNetAddrRange names;

	if (!addrs.parse(TheOpts.theAddrStr))
		cerr << here << "malformed address range: `" <<  TheOpts.theAddrStr << "'" << endl << xexit;

	if (!names.parse(TheOpts.theNameStr))
		cerr << here << "malformed name range: `" <<  TheOpts.theNameStr << "'" << endl << xexit;

	NetAddrRangeSym as;
	as.range(new PglNetAddrRange(addrs));
	TheAddrs.add(as);

	NetAddrRangeSym ns;
	ns.range(new PglNetAddrRange(names));
	TheNames.add(ns);

	TheZoneName = TheOpts.theZoneName;

	if (!TheZoneName)
		cerr << "missing zone name (use --zone option)" << endl << xexit;
}

static
void configure() {
	configureLogs(2);
	
	if (TheOpts.theCfgName)
		configureViaPgl();
	else
		configureViaCmdLine();

	TheZoneOrRoot = TheZoneName == "." ? "root" : TheZoneName;
	TheNameServerName = "ns." + TheZoneOrRoot + ".";
	// XXX: extract addresses from Robot.dns_resolver if possible
	TheNameServerIp = "172.16.101.100";

	if (TheAddrs.count() > TheNames.count()) {
		cerr << here << "warning: the number of IP addresses (" <<
			TheAddrs.count() << ") exceeds the number of domain names (" <<
			TheNames.count() << "); ignoring extra IPs" << endl;
	}
}

static
void printSoa(ostream &os) {
	const String serialNo = "200412021"; // XXX;

	os << "$TTL 3600" << endl << endl;

	os << "@ IN SOA "
		<< TheNameServerName
		<< " dnsmaster." << TheZoneOrRoot << '.' // person responsible
		<< " (" << endl
		<< "\t " << serialNo << "; Serial" << endl
		<< "\t 10800   ; Refresh" << endl
		<< "\t 1801    ; Retry" << endl
		<< "\t 3600000 ; Expire" << endl
		<< "\t 259200  ; Minimum" << endl
		<< ")" << endl
		<< endl;

	// NS record
	os << "\tIN\tNS\t" << TheNameServerName << endl;
	os << endl;


}

static
String reverseAddr(const NetAddr &addr, int octets) {
	ostringstream os;
	for (int i = 0; i<octets; ++i) {
		if (i)
			os << '.';
		os << addr.roctet(i);
	}
	os << ends;
	return os.str().c_str();
}

static
String reverseAddr(const NetAddr &addr) {
	return reverseAddr(addr, addr.addrN().len());
}

static
String reverseName(const NetAddr &addr) {
	if (!addr.isDynamicName())
		return addr.addrA();

	return "w000000" + addr.addrA();
}

static
String forwardName(const NetAddr &addr) {
	if (TheZoneName == ".")
		return addr.addrA();

	const String &name = addr.addrA();
	if (const char *zone = name.str(TheZoneName)) {
		const int end = zone - name.cstr() - 1; // position of the dot
		if (TheZoneName == zone && end >= 0 && name[end] == '.') {
			return (addr.isDynamicName() ? "*" : "") + addr.addrA()(0, end);
		}
	}

	cerr << here << "warning: " << name << " does not end with zone suffix ("
		<< TheZoneName << "), but probably should" << endl;

	return name;
}

static
String addressType(const InAddress &ina) {
	static const String A = "A";
	static const String AAAA = "AAAA";
	if (AF_INET6 == ina.family())
		return AAAA;
	return A;
}

static
String reverseZone(const InAddress &ina) {
	static const String INADDRARPA = "IN-ADDR.ARPA";
	static const String IP6INT = "IP6.INT";
	if (AF_INET6 == ina.family())
		return IP6INT;
	return INADDRARPA;
}

int main(int argc, char *argv[]) {

	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate())
		return -1;

	configure();

	const String dirFname(TheZoneOrRoot);
	const String revFname(TheZoneOrRoot + ".rev");
	ofstream direct(dirFname.cstr());
	ofstream reverse(revFname.cstr());

	NetAddr commonAddr;
	const String commonName = TheZoneName;

	// SOA records
	printSoa(direct);
	printSoa(reverse);

	// nameserver IP (only goes in to direct zone)
	direct << TheNameServerName << "\tIN\tA\t" << TheNameServerIp << endl;
	direct << endl;

	// A and PTR records
	for (int n = 0, a = 0; n < TheNames.count(); ++n, ++a) {
		if (a == TheAddrs.count())
			a = 0;
		const NetAddrSym &as = (NetAddrSym&)TheAddrs.item(a)->cast("addr");
		const NetAddrSym &ns = (NetAddrSym&)TheNames.item(n)->cast("addr");
		const NetAddr addr(as.val().addrN(), -1); // remove port number
		const NetAddr name(ns.val().addrA(), -1); // remove port number

		if (!n)
			commonAddr = addr;

		const char *pfx = "";
		if (name.addrA().chr('_')) {
			pfx = ";"; // comment
			cerr << here << "error: skipping invalid name '" << name << "' " <<
				"because underscores are not allowed in DNS" << endl;
		}

		direct << pfx
			<< forwardName(name)
			<< "\t\tIN\t"
			<< addressType(as.val().addrN())
			<< "\t"
			<< as.val().addrN().rawImage()
			<< endl;

		reverse << pfx
			<< reverseAddr(addr)
			<< "." << reverseZone(addr.addrN()) << "."
			<< "\t\tIN\tPTR\t" << reverseName(name) << '.'  << endl;
	}

	cout << "# BIND configuration for named.conf is below " << endl
		<< "# zone files are " << revFname << " and " << dirFname << endl
		<< endl;


	cout << "options {" << endl
		<< "\tdirectory \"/etc/namedb\";" << endl
		<< "\trecursion no;" << endl
		<< "};" << endl
		<< endl;

	cout << "zone \""
		// Just put the reverse zone name in the config file, without
		// and leading octets.
		// << reverseAddr(commonAddr, 2) << "."
		<< reverseZone(commonAddr.addrN()) << "\" {" << endl
		<< "\ttype master;" << endl
		<< "\tfile \"" << revFname << "\";" << endl
		<< "};" << endl
		<< endl;

	cout << "zone \"" << commonName << "\" {" << endl
		<< "\ttype master;" << endl
		<< "\tfile \"" << dirFname << "\";" << endl
		<< "};" << endl
		<< endl;

	return 0;
}
