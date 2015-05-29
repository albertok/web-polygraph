
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_ANYBODYPARSER_H
#define POLYGRAPH__CLIENT_ANYBODYPARSER_H

#include "client/BodyParserFarm.h"
#include "client/BodyParser.h"


// ignores any content
class AnyBodyParser: public BodyParser {
	public:
		static BodyParser *GetOne(CltXact *anOwner);

	public:
		virtual BodyParserFarm &farm() const;

	protected:
		virtual Size parse(const ParseBuffer &data);

	private:
		static BodyParserFarmT<AnyBodyParser> TheParsers;
};

#endif
