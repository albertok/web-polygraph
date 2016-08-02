
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_HOTSET_H
#define POLYGRAPH__RUNTIME_HOTSET_H

// the algorithm for computing next HotSet position
// and the storage for that postion
class HotSet {
	public:
		HotSet();

		void reset();

		Counter pos() const { return thePos; }
		void syncPos(const Counter size, const Counter wss);

		void pos(const Counter aPos) { thePos = aPos; }

	protected:
		Counter thePos;
};

#endif
