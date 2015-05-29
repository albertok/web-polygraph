
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "loganalyzers/InfoScope.h"
#include "loganalyzers/InfoScopes.h"


InfoScopes::InfoScopes() {
}

InfoScopes::~InfoScopes() {
	while (theScopes.count())
		delete theScopes.pop();
}

const InfoScope *InfoScopes::find(const String &image) const {
	int idx = -1;
	if (theIndex.find(image, idx))
		return scope(idx);
	else
		return 0;
}

void InfoScopes::add(const InfoScope &scope) {
	InfoScope *clone = new InfoScope(scope);
	absorb(clone);
}

void InfoScopes::absorb(InfoScope *&scope) {
	const String image = scope->image();
	int idx = -1;
	if (theIndex.find(image, idx))
		theIndex.valAt(idx) = count(); // store last index
	else
		theIndex.add(image, count());
	theScopes.append(scope);
	scope = 0;
}
