
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

		int pos() const { return thePos; }
		void syncPos(int size, int wss);

		void pos(int aPos) { thePos = aPos; }

	protected:
		int thePos;
};

#endif
