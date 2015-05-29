
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_INFOSCOPES_H
#define POLYGRAPH__LOGANALYZERS_INFOSCOPES_H

#include "xstd/String.h"
#include "xstd/Map.h"

class InfoScope;

// manages a collection of scopes indexed by their image
class InfoScopes {
	public:
		InfoScopes();
		~InfoScopes();

		int count() const { return theScopes.count(); }
		const InfoScope *scope(int idx) const { return theScopes[idx]; }
		const InfoScope *operator [](int idx) const { return scope(idx); }

		const InfoScope *find(const String &image) const;

		void add(const InfoScope &scope);
		void absorb(InfoScope *&scope);

	protected:
		Array<InfoScope*> theScopes;
		Map<int> theIndex;
};

#endif
