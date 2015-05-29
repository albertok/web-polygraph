
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_FNVHASH_H
#define POLYGRAPH__XSTD_FNVHASH_H

// Fowler-Noll-Vo hash, see http://www.isthe.com/chongo/tech/comp/fnv

const unsigned int FNV32OffsetBasics = 2166136261U;
const unsigned int FNV32Prime  = 16777619U; // 2^24 + 2^8 + 0x93

template <class T>
unsigned int FNV32Hash(const T &data, unsigned int hash = FNV32OffsetBasics) {
	for (unsigned int i = 0; i < sizeof(data); ++i) {
		const unsigned char byte =
			reinterpret_cast<const unsigned char *>(&data)[i];
		hash *= FNV32Prime;
		hash ^= byte;
	}
	return hash;
}

template <>
inline unsigned int FNV32Hash(const int &data, unsigned int hash) {
	hash *= FNV32Prime;
	hash ^= data;
	return hash;
}

#endif
