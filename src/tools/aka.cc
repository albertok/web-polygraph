
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <ctype.h>
#include "xstd/h/net/if.h"
#include "xstd/h/iostream.h"
#include "xstd/h/iomanip.h"

#include "xstd/NetIface.h"
#include "xstd/gadgets.h"
#include "base/CmdLine.h"
#include "base/opts.h"
#include "base/polyOpts.h"
#include "pgl/PglNetAddrRange.h"


class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,     "help",         "list of options"),
			theVersOpt(this,     "version",      "package version info"),
			theIfName(this,      "if <str>",     "interface name"),
			theAliases(this,     "aliases <str>","ip(s) or ip range(s) to set as aliases")
			{}

		virtual bool validate() const;

		virtual ostream &printAnonym(ostream &os) const;
		virtual bool parseAnonym(const Array<const char *> &opts);
		virtual bool canParseAnonym() const { return true; }

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theIfName;
		StrArrOpt theAliases;
};


static MyOpts TheOpts;
static NetIface *TheIface = 0;

/* MyOpt */

ostream &MyOpts::printAnonym(ostream &os) const {
	return os << "[interface_name] [alias] ...";
}

bool MyOpts::parseAnonym(const Array<const char *> &opts) {
	if (!opts.count())
		return true;

	int aliasStart = 0;
	if (!isdigit(*opts[0])) {
		theIfName.val(opts[0]);
		aliasStart = 1;
	}

	for (int i = aliasStart; i < opts.count(); ++i)
		theAliases.addItem(opts[i]);

	return true;
}

bool MyOpts::validate() const {
	if (!theIfName)
		cerr << "interface name is not specified" << endl;
	else
		return true;
	return false;
}

static
void configureLogs(int prec) {
	configureStream(cout, prec);
	configureStream(cerr, prec);
	configureStream(clog, prec);
}

static
void configure() {
	Socket::Configure();
	configureLogs(2);
	TheIface = new NetIface;
	TheIface->name(TheOpts.theIfName);
}

static
void getPrimaryAddress() {
	NetIface::Primaries addrs;
	if (!TheIface->primaries(addrs)) {
		clog << TheIface->name() << ": cannot get primary address(es) of " <<
			"the network interface" << endl;
		return;
	}
	if (addrs.vFour) {
		clog << TheIface->name() << ": primary IPv4 address is " << 
			addrs.vFour << endl;
	}
	if (addrs.vSix) {
		clog << TheIface->name() << ": primary IPv6 address is " << 
			addrs.vSix << endl;
	}
}

static
void delOldAliases() {
	const int delCount = TheIface->delAliases();
	Must(delCount >= 0);
	clog << TheIface->name() << ": deleted "	<< delCount << " old aliases" << endl;
}

static
void addNewAliases(const Array<NetAddr*> &addrs, const InAddress &netmask) {
	Array<InAddress> aliases(addrs.count());
	for (int i = 0; i < addrs.count(); ++i)
		aliases.append(addrs[i]->addrN());
	Must(TheIface->addAliases(aliases, netmask));

	static int newCount = 0;
	newCount += aliases.count();
	clog << TheIface->name() << ": created "
		<< aliases.count() << " new aliases " 
		<< "( " << newCount << " total )"
		<< endl;
}

int main(int argc, char *argv[]) {

	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate())
		return -1;

	configure();

	clog << TheOpts.theIfName << ": ";
	if (TheOpts.theAliases)
		TheOpts.theAliases.report(clog << "reseting aliases to ");
	else
		clog << "no new alias specified; will just delete old ones.";
	clog << endl;

	getPrimaryAddress();
	delOldAliases();

	for (int i = 0; i < TheOpts.theAliases.val().count(); ++i) {
		const String &alias = *TheOpts.theAliases.val()[i];

		PglNetAddrRange aliasParser;
		if (!aliasParser.parse(alias)) {
			cerr << TheOpts.theIfName << ": malformed alias: `" << alias << "' alias" << endl;
			return -2;
		}
		Array<NetAddr*> aliases;
		aliasParser.toAddrs(aliases);
		for (int i = 0; i < aliases.count(); ++i) {
			if (!aliases[i]->addrN().known()) {
				cerr << "error: can't parse alias " << aliases[i]->addrA() << endl;
				return -1;
			}
		}

		InAddress netmask;
		aliasParser.netmask(netmask);
		addNewAliases(aliases, netmask);
	}

	return 0;
}
