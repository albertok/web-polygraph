
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__LOGANALYZERS_INFOSCOPEDIM_H
#define POLYGRAPH__LOGANALYZERS_INFOSCOPEDIM_H

#include "xstd/String.h"
#include "xstd/Array.h"

class String;
class InfoScope;

// manages one dimension of InfoScope (e.g., phase dimention or side dimension)
class InfoScopeDim {
	friend class InfoScope;

	public:
		InfoScopeDim(const String &aLabel);
		InfoScopeDim(const InfoScopeDim &s);
		~InfoScopeDim();

		int count() const { return theNames.count(); }
		const Array<String*> &names() const { return theNames; }
		String image() const;

		void add(const String &name);
		void addFront(const String &name); // a hack for InfoScope::addSide()
		bool has(const String &name) const;

	protected:
		void reset();
		void copy(const InfoScopeDim &s);

	protected:
		String theLabel;
		Array<String*> theNames;
};

#endif
