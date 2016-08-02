
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include <limits.h>
#include <stdlib.h>

#include "xstd/Rnd.h"
#include "xstd/String.h"
#include "xstd/gadgets.h"
#include "base/RndPermut.h"
#include "base/polyVersion.h"


static String ThePrgName;

static
int uncorr(int n) {
	//int exp;
	//const double x = INT_MAX * (2*frexp(::sin((double)n), &exp) - 1);
	const double x = INT_MAX * ::sin(n*(double)n);
	const double y = fabs(x);
	return (int)Min(y, (double)INT_MAX);
}

static
int usage(std::ostream &os) {
	(void)PolyVersion();
	os << "Usage: " << ThePrgName << " <number_of_samples>" << endl;
	return 0;
}

int main(int argc, char *argv[]) {
	ThePrgName = argv[0];

	if (argc == 2 && String("--help") == argv[1])
		return usage(cout);

	int sampleCount = -1;
	if (argc != 2 || !isInt(argv[1], sampleCount)) {
		usage(cerr);
		return -1;
	}

	RndGen rngCont;
	RndGen rngSeeded;
	RndGen rngPermut;

	for (int i = 1; i <= sampleCount; ++i) {
		rngSeeded.seed(i);
		rngPermut.seed(LclPermut(i));
		cout
			<< ' ' << rngCont.trial32u()
			<< ' ' << rngSeeded.trial32u()
			<< ' ' << rngPermut.trial32u()
			<< ' ' << uncorr(i)
			<< endl;
	}

	return 0;
}
