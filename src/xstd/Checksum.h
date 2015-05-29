
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_CHECKSUM_H
#define POLYGRAPH__XSTD_CHECKSUM_H

#include "xstd/Size.h"

namespace xstd {

class ChecksumAlg;

// checksum storage
class Checksum {
	public:
		friend class ChecksumAlg;

	public:
		Checksum() { reset(); }

		void reset() { isSet = false; }

		bool set() const { return isSet; }
		bool equal(const Checksum &s) const;

		const char *image() const { return isSet ? (char*)theDigest : 0; }
		Size size() const { return SizeOf(theDigest); }

		char *buf() { return (char*)theDigest; }
		void set(bool be) { isSet = be; }

		ostream &print(ostream &os) const;

	protected:
		unsigned char theDigest[16];
		bool isSet;
};


// [incrementally] computes an MD5 (RFC 1321) 128 bit checksum; 
// we can make this interface more generic (e.g., add CRC32) when needed
class ChecksumAlg {
	public:
		ChecksumAlg();

		void reset();

		const Checksum &sum() const { return theSum; }

		// do not forgetr to call final() after all update()s!
		void update(const char *buf, Size size);
		void final();

	protected:
		unsigned char *digest() { return theSum.theDigest; }

		void update(const unsigned char *buf, unsigned bufLen);
		void transform(const unsigned char block[64]);
		void encode(unsigned char *output, const unsigned int *input, unsigned int len);
		void decode(unsigned int *output, const unsigned char *input, unsigned int len);

	protected:
		static const unsigned char ThePadding[64];

		unsigned int theState[4];
		unsigned int theCounts[4];
		unsigned char theBuffer[64]; // input buffer

		Checksum theSum;
};

} // namespace xstd

#endif
