
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "base/polygraph.h"

#include "client/ParseBuffer.h"
#include "client/CltXact.h"
#include "client/AnyBodyParser.h"


BodyParserFarmT<AnyBodyParser> AnyBodyParser::TheParsers;


BodyParser *AnyBodyParser::GetOne(CltXact *owner) {
	if (!TheParsers.capacity())
		TheParsers.limit(1024);

	BodyParser *p = TheParsers.get();
	p->configure(owner);
	return p;
}

BodyParserFarm &AnyBodyParser::farm() const {
	return TheParsers;
}

Size AnyBodyParser::parse(const ParseBuffer &data) {
	if (theOwner)
		theOwner->noteContent(data);
	return data.size();
}
