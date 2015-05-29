
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__CLIENT_MARKUPBODYPARSER_H
#define POLYGRAPH__CLIENT_MARKUPBODYPARSER_H

#include "client/BodyParserFarm.h"
#include "client/BodyParser.h"

class CltXact;
class CltCfg;

// searches for embedded objects and other interesting tags and
// forwards them to the owner
class MarkupBodyParser: public BodyParser {
	public:
		static BodyParser *GetOne(CltXact *anOwner, const CltCfg *aCfg);

	public:
		MarkupBodyParser();

		void configure(CltXact *anOwner, const CltCfg *aCfg);

		virtual void reset();
		virtual BodyParserFarm &farm() const;

		virtual void noteLeftovers(const ParseBuffer &data);
		virtual void noteOverflow(const ParseBuffer &data);

	protected:
		virtual Size parse(const ParseBuffer &data);
		void parseTag(const char *tagB, const char *tagE);

	protected:
		const CltCfg *theCfg;

	private:
		void resetSelf();

	private:
		static BodyParserFarmT<MarkupBodyParser> TheParsers;
};

#endif
