
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_PAGEINFO_H
#define POLYGRAPH__RUNTIME_PAGEINFO_H

#include "xstd/Time.h"

// web "page" information shared among all transactions fetching objects
// for the page
class PageInfo {
	public:
		static PageInfo *Create();
		static PageInfo *Use(PageInfo *info);
		static void Abandon(PageInfo *&info);

	public:
		bool loneUser() const;

		int useMax;
		Size size;    // cumulative
		Time start;
		Time lifeTime;

	protected:
		PageInfo();
		~PageInfo();

		bool used() const;

		void use();
		void reset();

	private:
		int theUseLevel;
		PageInfo *theNext;
		static PageInfo *TheFree;
};

#endif
