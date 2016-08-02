
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_ENDIAN_H
#define POLYGRAPH__XSTD_ENDIAN_H

#if HAVE_64BIT_BYTE_ORDER_CONVERSION

#if HAVE_ENDIAN_H
#include <endian.h>
#elif HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif

#else // !HAVE_64BIT_BYTE_ORDER_CONVERSION

#include "xstd/h/stdint.h"

inline uint64_t SwapEndian64(const uint64_t x) {
	return ((x & 0xff00000000000000ULL) >> 56) |
		((x & 0x00ff000000000000ULL) >> 40) |
		((x & 0x0000ff0000000000ULL) >> 24) |
		((x & 0x000000ff00000000ULL) >> 8) |
		((x & 0x00000000ff000000ULL) << 8) |
		((x & 0x0000000000ff0000ULL) << 24) |
		((x & 0x000000000000ff00ULL) << 40) |
		((x & 0x00000000000000ffULL) << 56);
}

#if WORDS_BIGENDIAN

inline uint64_t htobe64(uint64_t x) {
	return x;
}
inline uint64_t be64toh(uint64_t x) {
	return x;
}

#else // !WORDS_BIGENDIAN

inline uint64_t htobe64(uint64_t x) {
	return SwapEndian64(x);
}
inline uint64_t be64toh(uint64_t x) {
	return SwapEndian64(x);
}

#endif // !WORDS_BIGENDIAN

#endif // !HAVE_64BIT_BYTE_ORDER_CONVERSION

#endif // POLYGRAPH__XSTD_ENDIAN_H
