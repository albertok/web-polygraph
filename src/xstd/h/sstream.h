
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__XSTD_H_SSTREAM_H
#define POLYGRAPH__XSTD_H_SSTREAM_H

#if defined(HAVE_SSTREAM)
#   include <sstream>
#elif defined(HAVE_STRSTREAM)
#	include <stringstream>
#elif defined(HAVE_STRSTREAM_H)
#	include <stringstream.h>
#elif defined(HAVE_STRSTREA_H) /* MS file name limit */
#	include <strstrea.h>
#endif

// A streambuf with a fixed-size user-owned storage area.
// The implementation is incomplete and too forgiving, 
// but hopefully sufficient.
class fixedstreambuf: public std::streambuf {
	public:
		// based on http://www.dbforums.com/showthread.php?t=776452
		fixedstreambuf(char_type *buffer, std::streamsize size) {
			setp(buffer, buffer + size);
			setg(buffer, buffer, buffer + size);
		}

		// set position relative to dir
		virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir,
			std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) {

			pos_type result = pos_type(-1);

			if (mode & std::ios_base::out) {
				// convert to absolute offset if needed
				if (dir == std::ios_base::cur)
					off = pptr() - pbase() + off;
				else
				if (dir == std::ios_base::end)
					off = epptr() - pbase() - off;
				result = seekpos(off);
			}
			// XXX: the std::ios_base::in mode not supported yet
			return result;
		}

		// set absolute position (i.e., relative to the base)
		virtual pos_type seekpos(pos_type off,
			std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) {

			pos_type result = pos_type(-1);

			if (mode & std::ios_base::out) {
				if (0 <= off && off <= (epptr() - pbase())) {
					pbump(off - pos_type(pptr() - pbase()));
					result = off;
				}
			}
			// XXX: the std::ios_base::in mode not supported yet
			return result;
		} 
};

// an output stream that uses our fixedstreambuf
class ofixedstream: public std::ostream {
	public:
		ofixedstream(char_type *buffer, std::streamsize size):
			theBuffer(buffer, size) {
			this->init(&theBuffer); // set our custom buffer
		}

	private:
		fixedstreambuf theBuffer;
};

// More "fixed size stream buffer" ideas at
// http://www.inf.uni-konstanz.de/~kuehl/c++/iostream/
// http://www.velocityreviews.com/forums/showpost.php?p=1532770


// stringstream does not require freezing
inline
void streamFreeze(ostringstream &, bool) {
}

#endif
