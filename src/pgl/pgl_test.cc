
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xstd/gadgets.h"
#include "base/polyVersion.h"
#include "pgl/PglPp.h"
#include "pgl/PglParser.h"
#include "pgl/PglStaticSemx.h"


static String ThePrgName;

static
int usage(std::ostream &os) {
	(void)PolyVersion();
	os << "Usage: " << ThePrgName << " <input_file> [include_dir] ..." << endl;
	return 0;
}

int main(int argc, char **argv) {
	ThePrgName = argv[0];

	if (argc < 2)
		return usage(cerr);

	if (String("--help") == argv[1])
		return usage(cout);

	for (int i = 2; i < argc; ++i)
		PglPp::TheDirs.append(new String(argv[i]));

	PglPp pp(argv[1]);
	PglParser parser(&pp);

	if (const SynSym *s = parser.parse()) {
		PglStaticSemx semx;
		semx.interpret(*s);
		configureStream(cout, 2);
		s->print(cout, "");
		delete s;
		return 0;
	} else {
		cerr << here << "internal error: failed to interpret parsed " << 
			argv[1] << endl << xexit;
		return 2;
	}
}
