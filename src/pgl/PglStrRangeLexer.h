
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLSTRRANGELEXER_H
#define POLYGRAPH__PGL_PGLSTRRANGELEXER_H

#include "xstd/Ring.h"
#include "xstd/String.h"

class PglStrRange;

// parsers a string range specification into lexems
// and facilitates conversion of the lexems into PglStrRange blocks
class PglStrRangeLexer {
	protected:

		class Lexem {
			public:
				Lexem();
				Lexem(const char *aStart, const char *aStop);

				char tag() const { return *theStart; }
				const char *start() const { return theStart; }
				const char *stop() const { return theStop; }

			protected:
				const char *theStart; // pointers to the string
				const char *theStop;  // being parsed
		};

	public:
		PglStrRangeLexer(PglStrRange *anOwner, const String &anImage);

		operator void *() const { return theLexems.empty() ? 0 : (void*)-1; }

		bool parse(); // split the image into lexems

		// first char of the next lexem
		char next(int off = 0) const { return charAt(off); }
		// converts next lexem into a block
		void step();
		void skip(int count = 1);  // skips next count lexems

		bool range(bool isolated); // tries to convert next lexems into a range block

	protected:
		char charAt(int pos) const;
		int intAt(int pos) const;

		bool rangeItem(bool isolated);
		bool isDigit(char ch);

	protected:
		Ring<Lexem> theLexems;
		PglStrRange *theOwner;
		const String theImage;
};

#endif
