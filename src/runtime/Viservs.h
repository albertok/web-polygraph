
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_VISERVS_H
#define POLYGRAPH__RUNTIME_VISERVS_H

class HostCfg;
class NetAddr;
class ObjUniverse;

// iterates all visible servers, returning their HostCfg objects
class ViservIterator {
	public:
		ViservIterator();

		bool atEnd() const;
		int viserv() const;
		HostCfg *host() const;
		const NetAddr &addr() const;
		ObjUniverse *universe() const;

		ViservIterator &operator ++();
		void next();
	
	protected:
		void sync();

	protected:
		int theIdx;
};

#endif
