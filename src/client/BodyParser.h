
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_BODYPARSER_H
#define POLYGRAPH__CLIENT_BODYPARSER_H

#include "xstd/h/iosfwd.h"

class BodyParserFarm;
class ParseBuffer;
class CltXact;

// interface for various body parsers
class BodyParser {
	public:
		BodyParser();
		virtual ~BodyParser();

		void configure(CltXact *anOwner);

		virtual void reset();
		virtual BodyParserFarm &farm() const = 0;

		bool used() const { return isUsed; }

		Size noteData(const ParseBuffer &data);
		virtual void noteLeftovers(const ParseBuffer &data);
		virtual void noteOverflow(const ParseBuffer &data);
		
	protected:
		int logCat() const;
		virtual Size parse(const ParseBuffer &data) = 0;
		ostream &dumpContext(ostream &os, const char *ctxBeg, Size ctxSize) const;

	protected:
		CltXact *theOwner;
		bool isUsed;

	private:
		void resetSelf();
};

#endif
