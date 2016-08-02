
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__RUNTIME_IOBUF_H
#define POLYGRAPH__RUNTIME_IOBUF_H

#include "xstd/LibInit.h"

class IOBufState;
class RndBuf;

class IOBuf {
	public:
		IOBuf();
		IOBuf(Size aCapacity);
		~IOBuf();

		void reset() { theOutOff = theInOff = 0; if (theBuf) zip(); }

		bool empty() const { return theOutOff >= theInOff; }
		bool full() const { return theInOff >= theCapacity; }

		char *space() { if (!theBuf) unzip(); return theBuf + theInOff; }
		const char *content() const { if (theBuf) return theBuf + theOutOff; Assert(empty()); return 0; }

		Size capacity() const { return theCapacity; }
		Size spaceSize() const { return theCapacity - theInOff; }
		Size contSize() const { return theInOff - theOutOff; }

		void copyContent(IOBuf &buf, Size maxSize) const;

		void append(const char *buf, Size sz);

		/* appending random content (of the given kind) */
		void appendRnd(Size rndOff, Size sz, const RndBuf &rb); // exactly sz
		Size appendRndUpTo(Size rndOff, Size sz, const RndBuf &rb); // up to sz

		void appended(Size sz) { theInOff += sz; Assert(theInOff <= theCapacity); if (!theBuf) unzip(); }
		void consumed(Size sz) { theOutOff += sz; if (theOutOff == theInOff) reset(); else Assert(theOutOff < theInOff); }
		void consumedAll() { reset(); }
		void pack();

		// note: these two methods are not compatible with pack() or reset()!
		void saveState(IOBufState &state) const;
		void restoreState(const IOBufState &state);

		// used by appendRnd* methods and end users
		static Size RandomOffset(Size seed, Size off);
		static void RandomFill(ostream &os, Size rndOff, Size sz, const RndBuf &rndBuf);

	protected:
		void zip();
		void unzip();

	protected:
		char *theBuf;
		Size theCapacity;
		Size theOutOff;
		Size theInOff;
		bool isZippable;
};


class RdBuf: public IOBuf {
	public:
		RdBuf() {}
		RdBuf(Size aCapacity): IOBuf(aCapacity) {}
};

class WrBuf: public IOBuf {
	public:
		WrBuf() {}
		WrBuf(Size aCapacity): IOBuf(aCapacity) {}

		void overwrite(int offset, const char *buf, Size sz);
};

// to save/restore buffer state during I/O operations
class IOBufState {
	public:
		char *theBuf;
		Size theCapacity;
		Size theOutOff;
		Size theInOff;
};


// two kinds of random content are available
const RndBuf &RndText(); // an ASCII subset
const RndBuf &RndBinary(); // nearly all possible characters; includes RndText


LIB_INITIALIZER(IOBufInit)

#endif
