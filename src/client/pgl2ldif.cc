
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <fstream>
#include "xstd/h/iomanip.h"

#include "xstd/gadgets.h"
#include "base/RndPermut.h"
#include "base/opts.h"
#include "base/polyOpts.h"
#include "base/CmdLine.h"
#include "base/macros.h"
#include "base/AnyToString.h"
#include "pgl/PglPp.h"
#include "pgl/PglCtx.h"
#include "pgl/PglStaticSemx.h"
#include "pgl/PglStringSym.h"
#include "pgl/MembershipMapSym.h"
#include "pgl/AgentSymIter.h"
#include "pgl/RobotSym.h"
#include "runtime/UserCred.h"
#include "client/MembershipMap.h"


class MyOpts: public OptGrp {
	public:
		MyOpts():
			theHelpOpt(this,    "help",          "list of options"),
			theVersOpt(this,    "version",       "package version info"),
			theTemplate(this,   "template <filename>",  "LDIF template"),
			theCfgName(this,    "config <filename>",  "PGL configuration"),
			theCfgDirs(this,    "cfg_dirs <dirs>",  "directories for PGL #includes"),
			avoidDups(this,    "avoid_duplicates <yes|no>", "[slowly] avoid duplicate LDIF records", false),
			theGlbRngSeed(this, "global_rng_seed <int>","per-test r.n.g. seed", 1),
			theWorkerId(this,   "worker <int>", "SMP worker ID", 0)
			{}

		virtual bool validate() const;
		virtual String ExpandMacros(const Opt &opt, const String &str) const;

		//virtual ostream &printAnonym(ostream &os) const;
		//virtual bool parseAnonym(const Array<const char *> &opts);
		//virtual bool canParseAnonym() const { return true; }

	public:
		HelpOpt theHelpOpt;
		VersionOpt theVersOpt;
		StrOpt theTemplate;
		StrOpt theCfgName;
		StrArrOpt theCfgDirs;
		BoolOpt avoidDups;
		IntOpt theGlbRngSeed;
		IntOpt theWorkerId;
};


// empty-line-separated blob from LDIF template
class LdifTemplate {
	protected:
		struct Part {
			enum Type { ptNone = 0, ptPlain, ptGroup, ptUserName, ptUserPassword } type;
			String *image;

			Part(Type aType = ptNone, String *anImage = 0): type(aType), image(anImage) {}
		};

	public:
		LdifTemplate();
		~LdifTemplate();

		bool empty() const { return theParts.count() == 0; }
		bool groupDependent() const { return isGroupDependent; }
		bool userDependent() const { return isUserDependent; }

		String instantiate(const String &gname, const UserCred &creds) const;
		String instantiate(const String &gname) const;
		String instantiate() const;

		void load(istream &is);

	protected:
		String instantiate(const String *gname, const UserCred *creds) const;
		void addPlainPart(const char *beg, const char *end);
		void addPart(const Part &part);

	protected:
		Array<Part> theParts; // parts of the template
		bool isGroupDependent;
		bool isUserDependent;
};


static Array<MembershipMap*> TheMemberships;
static Array<String*> TheUsedUserGroupNames;
static Array<UserCred*> TheCredentials;
static int TheInstantiatedTemplatesCount = 0;
static MyOpts TheOpts;


/* MyOpt */

bool MyOpts::validate() const {
	if (theWorkerId.wasSet() && theWorkerId <= 0)
		cerr << "SMP worker ID must be positive; got: " << theWorkerId << endl;
	else
	if (!theTemplate)
		cerr << "must specify LDIF template file (--template)" << endl;
	else
	if (!theCfgName)
		cerr << "must specify PGL configuration file (--config)" << endl;
	else
		return true;
	return false;
}

String MyOpts::ExpandMacros(const Opt &opt, const String &str) const {
	if (opt.name() != "worker")
		return ExpandMacro(str, "%worker", AnyToString(theWorkerId));
	return str;
}

/* LdifTemplate */

LdifTemplate::LdifTemplate(): isGroupDependent(false), isUserDependent(false) {
}

LdifTemplate::~LdifTemplate() {
	while (theParts.count()) delete theParts.pop().image;
}

String LdifTemplate::instantiate(const String *gname, const UserCred *creds) const {
	// split creds into uname and password
	const char *password = 0;
	int unameLen = 0;
	if (creds) {
		unameLen = creds->name().size();
		password = creds->password().data();
	}

	String res;
	for (int i = 0; i < theParts.count(); ++i) {
		const Part &part = theParts[i];
		switch (part.type) {
			case Part::ptPlain:
				Assert(part.image);
				res += *part.image;
				break;
			case Part::ptGroup:
				Assert(gname);
				res += *gname;
				break;
			case Part::ptUserName:
				Assert(creds);
				res.append(creds->image().data(), unameLen);
				break;
			case Part::ptUserPassword:
				Assert(creds);
				res += password;
				break;
			default:
				Assert(false);
		}
	}

	return res;
}

String LdifTemplate::instantiate(const String &gname, const UserCred &creds) const {
	return instantiate(&gname, &creds);
}

String LdifTemplate::instantiate(const String &gname) const {
	return instantiate(&gname, 0);
}

String LdifTemplate::instantiate() const {
	return instantiate(0, 0);
}

void LdifTemplate::load(istream &is) {
	String buf;
	char c;
	while (is.read(&c, 1)) {
		const bool atEol = !buf || buf.last() == '\n';
		buf += c;
		if (c == '\n' && atEol)
			break;
	}

	if (!buf)
		return;

	const char *p = buf.cstr();
	while (const char *pholder = strchr(p, '{')) {
		if (strncmp(pholder, "{group}", 7) == 0) {
			isGroupDependent = true;
			addPlainPart(p, pholder);
			addPart(Part(Part::ptGroup));
		} else
		if (strncmp(pholder, "{username}", 10) == 0) {
			isUserDependent = true;
			addPlainPart(p, pholder);
			addPart(Part(Part::ptUserName));
		} else
		if (strncmp(pholder, "{password}", 10) == 0) {
			isUserDependent = true;
			addPlainPart(p, pholder);
			addPart(Part(Part::ptUserPassword));
		} else {
			cerr << "unknown placeholder near `";
			const char *eop = strchr(pholder, '}');
			eop = eop ? eop+1 : (pholder + Min((int)strlen(pholder), 10));
			cerr.write(pholder, eop - pholder);
			exit(1);
		}
		p = strchr(pholder, '}') + 1;
	}
	addPlainPart(p, p + strlen(p));
}

void LdifTemplate::addPlainPart(const char *beg, const char *end) {
	if (end > beg) {
		String *image = new String(beg, end - beg);
		addPart(Part(Part::ptPlain, image));
	}
}

void LdifTemplate::addPart(const Part &part) {
	theParts.append(part);
}

static
void noteBlob(const String &blob) {
	if (TheOpts.avoidDups) {
		static Map<int> blobsSeen;
		if (int *count = blobsSeen.valp(blob)) {
			(*count)++;
			return;
		}
		blobsSeen.add(blob, 1);
	}
	cout << blob << endl;
	TheInstantiatedTemplatesCount++;
}

static
void userDependentStep(const LdifTemplate &tmp) {
	for (int u = 0; u < TheCredentials.count(); ++u) {
		const UserCred &credentials = *TheCredentials[u];
		int matchCount = 0;
		for (int g = 0; g < TheMemberships.count(); ++g) {
			for (MembershipMap::GroupIterator i =
				TheMemberships[g]->groupIterator(credentials); i; ++i) {
				noteBlob(tmp.instantiate(*i, credentials));
				++matchCount;
			}
		}
		if (!matchCount) {
			cerr << "warning: user " << credentials.image() << 
				" belongs to no group " <<
				"and will not have LDIF records" << endl;
		}
	}
}

static
void groupDependentStep(const LdifTemplate &tmp) {
	for (int g = 0; g < TheUsedUserGroupNames.count(); ++g) {
		noteBlob(tmp.instantiate(*TheUsedUserGroupNames[g]));
	}
}

static
void step(const LdifTemplate &tmp) {
	if (tmp.userDependent()) {
		userDependentStep(tmp);
	} else 
	if (tmp.groupDependent()) {
		groupDependentStep(tmp);
	} else {
		noteBlob(tmp.instantiate());
	}
}

template <class Stats>
static
void logStats(const String &label, const Stats &stats) {
	const int padding = Max(0, 30 - label.len());
	clog << "fyi: " << label << ": " << setw(padding) << " " 
		<< setw(10) << stats << endl;
}

int main(int argc, char **argv) {
	CmdLine cmd;
	cmd.configure(Array<OptGrp*>() << &TheOpts);
	if (!cmd.parse(argc, argv) || !TheOpts.validate())
		return -1;

	configureStream(cout, 2);
	configureStream(clog, 3);

	PglStaticSemx::WorkerId(TheOpts.theWorkerId);

	// set random seeds
	GlbPermut().reseed(TheOpts.theGlbRngSeed);

	// parse templates
	ifstream f(TheOpts.theTemplate.cstr());
	Array<LdifTemplate*> templates;
	while (f) {
		LdifTemplate *tmp = new LdifTemplate();
		tmp->load(f);
		if (!tmp->empty())
			templates.append(tmp);
	}
	logStats("templates", templates.count());

	if (!templates.count())
		cerr << TheOpts.theTemplate << ": warning: no templates found" << endl;

	// parse PGL
	TheOpts.theCfgDirs.copy(PglPp::TheDirs);
	PglStaticSemx::Interpret(TheOpts.theCfgName);
	logStats("use()d MembershipMaps", PglStaticSemx::TheMembershipsToUse.count());

	if (!PglStaticSemx::TheMembershipsToUse.count())
		cerr << TheOpts.theCfgName << ": warning: no MembershipMaps use()d" << endl;

	long groupSpace = 0;
	long userSpace = 0;
	for (int i = 0; i < PglStaticSemx::TheMembershipsToUse.count(); ++i) {
		MembershipMap *g = new MembershipMap;
		g->configure(*PglStaticSemx::TheMembershipsToUse[i], i+1);
		groupSpace += g->groupNameCount();
		userSpace += g->userNameCount();
		TheMemberships.append(g);
	}
	logStats("possible user group names", groupSpace);
	logStats("possible user credentials", userSpace);

	for (AgentSymIter u(PglStaticSemx::TheAgentsToUse, RobotSym::TheType, false); u; ++u) {
		const RobotSym &r = (const RobotSym&)u.agent()->cast(RobotSym::TheType);
		Array<String*> creds;
		r.credentials(creds);
		for (int c = 0; c < creds.count(); ++c) {
			TheCredentials.append(new UserCred(*creds[c]));
		}
	}
	logStats("user credentials", TheCredentials.count());

	for (int g = 0; g < TheMemberships.count(); ++g) {
		TheMemberships[g]->collectUsedGroupNames(TheCredentials, 
			TheUsedUserGroupNames);
	}
	logStats("user groups", TheUsedUserGroupNames.count());
	//logStats("mean groups per user", TheUsedUserGroupNames.count()/(double)TheCredentials.count());
	//logStats("mean users per group", TheCredentials.count()/(double)TheUsedUserGroupNames.count());

	for (int t = 0; t < templates.count(); ++t)
		step(*templates[t]);
	logStats("instantiated templates", TheInstantiatedTemplatesCount);

	// cleanup
	while (templates.count()) delete templates.pop();
	while (TheUsedUserGroupNames.count()) delete TheUsedUserGroupNames.pop();

	return 0;
}
